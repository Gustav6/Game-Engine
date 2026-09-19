#include "GraphicsEngine.pch.h"
#include "GraphicsEngine.hpp"

#include "CommonUtilities/Collision/Frustum.hpp"
#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Vector4.hpp"

#include "Objects/Mesh.h"
#include "Objects/Vertex.h"
#include "Objects/Light.h"
#include "Objects/Texture.h"
#include "Objects/Shader.h"
#include "Objects/Sampler.h"
#include "Objects/Gbuffer.h"
#include "Objects/Camera.h"
#include "Objects/Font.h"
#include "Objects/Text.h"

#include "ConstantBuffers/FrameBuffer.h"
#include "ConstantBuffers/ObjectBuffer.h"
#include "ConstantBuffers/AnimationBuffer.h"
#include "ConstantBuffers/LightBuffer.h"
#include "ConstantBuffers/CascadeBuffer.h"

#include "RHI/GraphicsCommandList.h"
#include "RHI/RHIShaderReflectionInfo.h"
#include "RHI/PipelineStateObject.h"
#include "RHI/RenderHardwareInterface.h"

#include "Materials/Material.h"
#include "Materials/MaterialHelpers.h"
#include "Materials/MaterialShaderIncludeHandler.h"

#include "Textures/Albedo/Default_C.h"
#include "Textures/Normal/Default_N.h"

#include "ShadowCube_GS.h"
#include "FullTexture_VS.h"
#include "BRDF_LUT_PS.h"
#include "GBuffer_PS.h"
#include "Surface_VS.h"
#include "Deferred_DirLight_PS.h"
#include "Deferred_SpotLight_PS.h"
#include "Deferred_PointLight_PS.h"
#include "Deferred_Ambient_PS.h"

#include "CommonUtilities/Json/JsonWrapper.h"

/*
* GraphicsEngine handles rendering of a scene.
* Handles culling using whatever cameras it's told to use.
* Renders onto a user-provided render target and depth.
*/

struct InternalRenderItem
{
    const DrawCall* DrawCall;
    unsigned int ElementIndex;
	unsigned int MaterialIndex;
};

GraphicsEngine::GraphicsEngine() = default;
GraphicsEngine::~GraphicsEngine() = default;

GraphicsEngine& GraphicsEngine::Get()
{
	static GraphicsEngine myInstance;
	return myInstance;
}

bool GraphicsEngine::Initialize(HWND aWindowHandle, const std::filesystem::path& aContentRoot)
{
	myContentRoot = aContentRoot;

	if (!std::filesystem::exists(myContentRoot))
	{
		GELOG(Fatal, "Content root dir is not a valid path! Provided root was {}.", myContentRoot.string());
		return false;
	}

	myShaderRoot = aContentRoot / "Shaders";
	
	if (!std::filesystem::exists(myShaderRoot))
	{
		GELOG(Fatal, "Shader root dir is not a valid path! Provided root was {}.", myShaderRoot.string());
		return false;
	}

	myRHI = std::make_unique<RenderHardwareInterface>();
	myBackBuffer = std::make_unique<Texture>();
	myDepthBuffer = std::make_unique<Texture>();

	if (!myRHI->Initialize(aWindowHandle, true, *myBackBuffer, *myDepthBuffer))
	{
		return false;
	}

	RegisterMaterialShaders();

	if (!InitializeBuiltinAssets())
	{
		return false;
	};

	if (!InitializeRenderResources())
	{
		return false;
	};

	return true;
}

void GraphicsEngine::Render(GraphicsCommandList& inoutCommandList, const Scene& aScene)
{
	inoutCommandList.BeginEvent("Reset Pipeline");
	{
		inoutCommandList.ClearRenderTargets(myGBuffer->GetTextures(), GBuffer::NUM_GBUFFER_TARGETS);
		inoutCommandList.ClearRenderTarget(*myBackBuffer);
		inoutCommandList.ClearDepthStencil(*myDepthBuffer);
	}
	inoutCommandList.EndEvent();

	std::vector<InternalRenderItem> opaqueItems;
	std::vector<InternalRenderItem> alphaItems;
	std::vector<InternalRenderItem> shadowItems;

	for (const auto& drawCall : aScene.DrawCalls)
	{
		for (unsigned int e = 0; e < drawCall.Mesh->myElements.size(); e++)
		{
			unsigned int materialIndex = drawCall.Mesh->myElements[e].MaterialIndex;

			const MaterialInterface* material = drawCall.Materials[materialIndex];

			if (material == nullptr)
			{
				material = myDefaultMaterial.get();
			}

			InternalRenderItem item =
			{
				.DrawCall = &drawCall,
				.ElementIndex = e,
				.MaterialIndex = materialIndex
			};

			switch (material->GetBlendMode())
			{
				case MaterialBlendMode::Opaque:
				{
					opaqueItems.push_back(item);
					break;
				}
				case MaterialBlendMode::Alpha:
				{
					alphaItems.push_back(item);
					break;
				}
				default:
				{
					GELOG(Error, "Mesh {} has a invalid Material Blend Mode setting!", drawCall.Mesh->GetName());
					break;
				}
			}

			shadowItems.push_back(item);
		}
	}

	CommonUtilities::Vector3f cameraPosition = aScene.SceneCamera.Transform.GetTranslation();

	// Front to Back
	std::sort(opaqueItems.begin(), opaqueItems.end(),
		[&](const InternalRenderItem& A, const InternalRenderItem& B) -> bool
		{
			float distanceToA = (A.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();
			float distanceToB = (B.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();

			return distanceToA < distanceToB;
		});

	// Back to Front
	std::sort(alphaItems.begin(), alphaItems.end(),
		[&](const InternalRenderItem& A, const InternalRenderItem& B) -> bool
		{
			float distanceToA = (A.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();
			float distanceToB = (B.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();

			return distanceToA > distanceToB;
		});

	{
		std::vector<Sampler*> samplerList(mySamplers.size());

		for (size_t s = 0; s < mySamplers.size(); s++)
		{
			samplerList[s] = mySamplers[s].get();
		}

		inoutCommandList.SetShaderSamplers(samplerList.data(), samplerList.size(), 0, PipelineStage_VertexShader | PipelineStage_PixelShader);
	}
	
	{
		inoutCommandList.SetShaderResource(myEnvironmentCubemap.get(), 98, PipelineStage_PixelShader);
		inoutCommandList.SetShaderResource(myLUTTexture.get(), 99, PipelineStage_PixelShader);
	}

	inoutCommandList.BeginEvent("Shadow Pass");
	{
		inoutCommandList.SetOverridePipelineState(*myShadowPSO, PipelineStage_None);

		RenderShadowMaps(inoutCommandList, aScene.SceneLights, aScene.SceneCamera, shadowItems);

		inoutCommandList.ClearOverridePipelineState();
	}
	inoutCommandList.EndEvent();

	FrameBuffer frameBuffer;
	frameBuffer.View = aScene.SceneCamera.Transform.GetFastInverse();
	frameBuffer.Projection = aScene.SceneCamera.RenderCamera->GetProjection();
	frameBuffer.CameraPosition = aScene.SceneCamera.Transform.GetTranslation();
	frameBuffer.TypeOfRenderPass = static_cast<unsigned int>(myCurrentRenderPass);

	UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	inoutCommandList.BeginEvent("Main Pass");
	{
		inoutCommandList.BeginEvent("Deffered Render");
		{
			inoutCommandList.BeginEvent("Object GBuffer Render");
			{
				inoutCommandList.SetRenderTargets(myGBuffer->GetTextures(), GBuffer::NUM_GBUFFER_TARGETS, myDepthBuffer.get());
				RenderDrawCalls(inoutCommandList, opaqueItems, DrawCallFlag_BindTextures | DrawCallFlag_BindMaterial);
			}
			inoutCommandList.EndEvent();

			inoutCommandList.BeginEvent("Light Fullscreen Render");
			{
				inoutCommandList.SetRenderTarget(myBackBuffer.get(), nullptr);

				// GBuffer set resources
				{
					std::vector<const Texture*> gBufferTextures;
					gBufferTextures.reserve(GBuffer::NUM_GBUFFER_TARGETS);

					for (size_t t = 0; t < GBuffer::NUM_GBUFFER_TARGETS; t++)
					{
						gBufferTextures.emplace_back(&myGBuffer->GetTextures()[t]);
					}

					inoutCommandList.SetShaderResources(gBufferTextures.data(), gBufferTextures.size(), 0, PipelineStage_PixelShader);
				}


				inoutCommandList.BeginEvent("Ambient Light");
				inoutCommandList.SetPipelineStateObject(*myDeferredAmbientPSO);
				inoutCommandList.Draw(4);
				inoutCommandList.EndEvent();

				if (myCurrentRenderPass == RenderPass::sRGB)
				{
					for (unsigned int l = 0; l < aScene.SceneLights.size(); l++)
					{
						if (aScene.SceneLights[l].LightData == nullptr)
						{
							continue;
						}

						LightBuffer lightBuffer;
						lightBuffer.Lights[0] = *aScene.SceneLights[l].LightData;
						UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::LightBuffer, lightBuffer, 3, PipelineStage_PixelShader);

						switch (aScene.SceneLights[l].LightData->Type)
						{
						case LightType::Directional:
						{
							inoutCommandList.BeginEvent("Directional Light");
							std::vector<const Texture*> dirLightShadowMaps;
							dirLightShadowMaps.reserve(NUM_CASCADES);

							for (size_t c = 0; c < NUM_CASCADES; c++)
							{
								dirLightShadowMaps.emplace_back(myCascades[c].ShadowMap.get());
							}

							inoutCommandList.SetShaderResources(dirLightShadowMaps.data(), dirLightShadowMaps.size(), 100, PipelineStage_PixelShader);
							inoutCommandList.SetPipelineStateObject(*myDeferredDirPSO);
							inoutCommandList.Draw(4);
							inoutCommandList.EndEvent();
							break;
						}
						case LightType::Spot:
						{
							inoutCommandList.BeginEvent("Spot Light");

							const Texture* shadowMap = aScene.SceneLights[l].ShadowMap;

							inoutCommandList.SetShaderResource(shadowMap, 104, PipelineStage_PixelShader);

							inoutCommandList.SetPipelineStateObject(*myDeferredSpotPSO);
							inoutCommandList.Draw(4);

							inoutCommandList.EndEvent();
							break;
						}
						case LightType::Point:
						{
							inoutCommandList.BeginEvent("Point Light");

							const Texture* shadowMap = aScene.SceneLights[l].ShadowMap;

							inoutCommandList.SetShaderResource(shadowMap, 112, PipelineStage_PixelShader);

							inoutCommandList.SetPipelineStateObject(*myDeferredPointPSO);
							inoutCommandList.Draw(4);

							inoutCommandList.EndEvent();
							break;
						}
						}
					}
				}
			}
			inoutCommandList.EndEvent();
		}
		inoutCommandList.EndEvent();

		inoutCommandList.BeginEvent("Alpha Render");
		{
			inoutCommandList.SetRenderTarget(myBackBuffer.get(), myDepthBuffer.get());
			UploadSceneLightingData(inoutCommandList, aScene.SceneLights);
			RenderDrawCalls(inoutCommandList, alphaItems, DrawCallFlag_BindTextures | DrawCallFlag_BindMaterial);
		}
		inoutCommandList.EndEvent();
	}
	inoutCommandList.EndEvent();

	inoutCommandList.BeginEvent("Render Screen space Objects");
	{
		inoutCommandList.SetRenderTarget(myBackBuffer.get(), nullptr);

		UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

		const CommonUtilities::Vector2ui windowSize = myRHI->GetClientSize();

		CommonUtilities::Matrix4x4f scale;
		scale.SetScaling({ 2.0f / static_cast<float>(windowSize.x), -2.0f / static_cast<float>(windowSize.y), 1.0f });

		CommonUtilities::Matrix4x4f translation;
		translation.SetTranslation({ -1.0f, 1.0f, 0.0f });

		const CommonUtilities::Matrix4x4f pixelToNDC = scale * translation;

		if (myText->myIsDirty)
		{
			myText->RebuildGeometry();
			myText->RebuildTextRenderMatrix();
			std::string vertexBufferName = std::format("{}_VX", myText->myName);
			myRHI->CreateVertexBuffer(vertexBufferName, myText->myVertices, myText->myVertexBuffer);
			std::string indexBufferName = std::format("{}_PX", myText->myName);
			myRHI->CreateIndexBuffer(indexBufferName, myText->myIndices, myText->myIndexBuffer);
			myText->myIsDirty = false;
		}

		ObjectBuffer objectBuffer;
		objectBuffer.World = myText->GetRenderMatrix() * pixelToNDC;
		objectBuffer.WorldInvT = objectBuffer.World.GetFastInverse().GetTranspose();

		UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader | PipelineStage_PixelShader);

		inoutCommandList.SetShaderResource(&myDefaultFont->myAtlas, 0, PipelineStage_PixelShader);

		inoutCommandList.SetPipelineStateObject(myText->myFontMaterial->myPSO);

		/*if (material->HasParameters())
		{
			if (material->IsMaterialDataDirty())
			{
				material->RefreshMaterialData();
			}

			UpdateAndSetConstantBufferInternal(inoutCommandList, ConstantBuffer::MaterialBuffer, material->GetParameterDataBlock(), Material::MATERIAL_BUFFER_SIZE, 5, PipelineStage_VertexShader | PipelineStage_PixelShader);
		}*/

		inoutCommandList.SetVertexBuffer(&myText->myVertexBuffer);
		inoutCommandList.SetIndexBuffer(&myText->myIndexBuffer);
		inoutCommandList.DrawIndexed(static_cast<unsigned int>(myText->myIndices.size()), 0);
	}
	inoutCommandList.EndEvent();
}

void GraphicsEngine::Present() const
{
	myRHI->Present();
}

void GraphicsEngine::CycleToNextRenderPass(int aDirection)
{
	int nextRenderPass = static_cast<int>(myCurrentRenderPass) + aDirection;

	if (nextRenderPass >= static_cast<int>(RenderPass::MAX)) 
	{
		nextRenderPass = 0;
	}
	else if (nextRenderPass < 0)
	{
		nextRenderPass = static_cast<int>(RenderPass::MAX) - 1;
	}

	myCurrentRenderPass = static_cast<RenderPass>(nextRenderPass);
}

bool GraphicsEngine::CreateConstantBuffer(ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize)
{
	return CreateConstantBufferInternal(aBufferID, aName, aBufferSize);
}

CommonUtilities::Vector2ui GraphicsEngine::GetClientSize() const
{
	return myRHI->GetClientSize();
}

bool GraphicsEngine::CreateCommandList(std::string_view aName, GraphicsCommandList& outCommandList) const
{
	return myRHI->CreateCommandList(aName, outCommandList);
}

void GraphicsEngine::ExecuteCommandList(const GraphicsCommandList& aCommandList) const
{
	myRHI->ExecuteCommandList(aCommandList);
}

bool GraphicsEngine::CreateMaterial(const MaterialDescription& aDescription, Material& outMaterial) const
{
	Shader materialVS;
	Shader materialPS;

	if (aDescription.BlendMode == MaterialBlendMode::None)
	{
		GELOG(Error, "Material {} has invalid Blend Mode setting!", aDescription.Name);
		return false;
	}

	if (aDescription.Domain == MaterialDomain::None)
	{
		GELOG(Error, "Material {} has invalid Material Domain setting!", aDescription.Name);
		return false;
	}

	if (aDescription.ShadingModel == ShadingModel::None)
	{
		GELOG(Error, "Material {} has invalid Shading Model setting!", aDescription.Name);
		return false;
	}

	if (aDescription.Name.empty())
	{
		GELOG(Error, "Material {} has no given name!", aDescription.Name);
		return false;
	}

	{
		std::filesystem::path shaderProgramPath;

		if (!aDescription.VertexShader.empty())
		{
			shaderProgramPath = aDescription.VertexShader;
		}
		else
		{
			shaderProgramPath = myMaterialDomainShaders.at(aDescription.Domain);
		}

		const std::filesystem::path shaderRoot = myShaderRoot / "Material" / "Include";
		std::filesystem::path materialProgramPath;

		if (aDescription.MaterialShaderCode.empty())
		{
			materialProgramPath = myShaderRoot / "Material" / "Include" / "Material.hlsli";
		}
		else
		{
			materialProgramPath = aDescription.MaterialShaderCode;
		}

		MaterialShaderIncludeHandler handler(shaderRoot, shaderProgramPath, materialProgramPath);

		if (!myRHI->CompileShader(ShaderType::VertexShader, shaderProgramPath, &handler, true, materialVS))
		{
			return false;
		}
	}

	{
		std::filesystem::path shaderProgramPath;

		if (!aDescription.PixelShader.empty())
		{
			shaderProgramPath = aDescription.PixelShader;
		}
		else
		{
			switch (aDescription.BlendMode)
			{
				case MaterialBlendMode::Opaque:
				{
					shaderProgramPath = myShaderRoot / "Material" / "GBuffer_PS.hlsl";
					break;
				}
				case MaterialBlendMode::Alpha:
				{
					shaderProgramPath = myMaterialShadingModelShaders.at(aDescription.ShadingModel);
					break;
				}
				default:
				{
					// TODO: Log error
					break;
				}
			}
		}

		const std::filesystem::path shaderRoot = myShaderRoot / "Material" / "Include";
		std::filesystem::path materialProgramPath;

		if (aDescription.MaterialShaderCode.empty())
		{
			materialProgramPath = myShaderRoot / "Material" / "Include" / "Material.hlsli";
		}
		else
		{
			materialProgramPath = aDescription.MaterialShaderCode;
		}

		MaterialShaderIncludeHandler handler(shaderRoot, shaderProgramPath, materialProgramPath);

		if (!myRHI->CompileShader(ShaderType::PixelShader, shaderProgramPath, &handler, true, materialPS))
		{
			return false;
		}
	}

	RHIShaderReflectionInfo vsInfo, psInfo;
	RHIShaderReflector::Reflect(materialVS.GetDataPtr(), materialVS.GetDataSize(), vsInfo);
	RHIShaderReflector::Reflect(materialPS.GetDataPtr(), materialPS.GetDataSize(), psInfo);

	const RHIShaderReflectionInfo* materialBufferSource = nullptr;
	static std::string materialBufferName = "MaterialBuffer";

	if (vsInfo.ConstantBufferNameToIndex.contains(materialBufferName))
	{
		materialBufferSource = &vsInfo;
	}
	else if (psInfo.ConstantBufferNameToIndex.contains(materialBufferName))
	{
		materialBufferSource = &psInfo;
	}

	memset(outMaterial.myData, 0, Material::MATERIAL_BUFFER_SIZE);

	if (materialBufferSource != nullptr)
	{
		const RHIShaderReflectionInfo::ConstantBufferInfo& info = materialBufferSource->ConstantBuffers[materialBufferSource->ConstantBufferNameToIndex.at(materialBufferName)];

		for (size_t i = 0; i < info.Members.size(); i++)
		{
			const auto& member = info.Members[i];

			MaterialParameterInfo param;
			param.Name = member.Name;
			param.Type = MaterialHelpers::HLSLTypeToMaterialParameterType(member.Type);
			param.Size = member.Size;
			param.Offset = member.Offset;
			param.Index = i;

			memcpy_s(outMaterial.myData + param.Offset, param.Size, member.Default, member.Size);

			outMaterial.myParameterNameToIndex.emplace(param.Name, static_cast<unsigned int>(outMaterial.myParameters.size()));
			outMaterial.myParameters.emplace_back(std::move(param));
		}
	}

	PipelineStateDescription matPSODesc;
	matPSODesc.Name = std::format("{}_MAT_PSO", aDescription.Name);
	matPSODesc.VertexShader.ByteCode = materialVS.GetDataPtr();
	matPSODesc.VertexShader.ByteCodeSize = materialVS.GetDataSize();
	matPSODesc.PixelShader.ByteCode = materialPS.GetDataPtr();
	matPSODesc.PixelShader.ByteCodeSize = materialPS.GetDataSize();
	matPSODesc.InputLayoutElements = Vertex::Description;
	matPSODesc.Topology = Topology::TriangleList;

	switch (aDescription.BlendMode)
	{
		case MaterialBlendMode::Opaque:
		{
			matPSODesc.BlendMode = BlendMode::Opaque;
			break;
		}
		case MaterialBlendMode::Alpha:
		{
			matPSODesc.BlendMode = BlendMode::Alpha;
			break;
		}
		default:
		{
			// TODO: Log error
			break;
		}
	}

	PipelineStateObject matPSO;

	if (!myRHI->CreatePipelineStateObject(matPSODesc, matPSO))
	{
		return false;
	}

	CreateMaterialTextureSlots(vsInfo, outMaterial);
	CreateMaterialTextureSlots(psInfo, outMaterial);
	
	outMaterial.myPSO = matPSO;
	outMaterial.myName = aDescription.Name;
	outMaterial.myDescription = aDescription;

	return true;
}

bool GraphicsEngine::CreateShadowMap(std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outShadowMap, bool aCube) const
{
	return myRHI->CreateDepthStencil(aName, aWidth, aHeight, outShadowMap, aCube);
}

bool GraphicsEngine::CreateTexture(std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture) const
{
	return myRHI->CreateTexture(aName, aWidth, aHeight, aNumMips, aFormat, aBindFlags, outTexture);
}

bool GraphicsEngine::CreateFontAsset(const std::filesystem::path& aFontPath, const std::filesystem::path& aFontAtlasPath, Font& outFont) const
{
	JsonData fontData = Json::Read(aFontPath);

	if (!fontData.IsValid())
	{
		GELOG(Error, "Failed to read font JSON [{}]!", aFontPath.string());
		return false;
	}

	JsonData atlasSettings = fontData.GetObjectValue("atlas");

	if (!atlasSettings.IsValid())
	{
		GELOG(Error, "Font JSON is missing [atlas]!");
		return false;
	}

	float width = static_cast<float>(atlasSettings.GetUnsigned("width"));
	float height = static_cast<float>(atlasSettings.GetUnsigned("height"));

	JsonData metricsSettings = fontData.GetObjectValue("metrics");

	if (!metricsSettings.IsValid())
	{
		GELOG(Error, "Font JSON is missing [metrics]!");
		return false;
	}

	outFont.myLineHeight = metricsSettings.GetFloat("lineHeight");
	outFont.myAscender = metricsSettings.GetFloat("ascender");
	outFont.myDescender = metricsSettings.GetFloat("descender");

	const std::vector<JsonData> glyphs = fontData.GetObjectValueArray("glyphs");

	if (glyphs.empty())
	{
		GELOG(Error, "Font JSON contains no glyphs [{}]!", aFontPath.string());
		return false;
	}

	for (const JsonData& glyphData : glyphs)
	{
		Font::Glyph glyph = {};

		glyph.UniCode = glyphData.GetUnsigned("unicode");
		glyph.Advance = glyphData.GetFloat("advance");

		if (glyphData.HasKey("planeBounds"))
		{
			JsonData planeBounds = glyphData.GetObjectValue("planeBounds");

			glyph.PlaneBounds.Left = planeBounds.GetFloat("left");
			glyph.PlaneBounds.Bottom = planeBounds.GetFloat("bottom");
			glyph.PlaneBounds.Right = planeBounds.GetFloat("right");
			glyph.PlaneBounds.Top = planeBounds.GetFloat("top");
		}

		if (glyphData.HasKey("atlasBounds"))
		{
			JsonData atlasBounds = glyphData.GetObjectValue("atlasBounds");

			glyph.AtlasBounds.Left = atlasBounds.GetFloat("left") / width;
			glyph.AtlasBounds.Bottom = atlasBounds.GetFloat("bottom") / height;
			glyph.AtlasBounds.Right = atlasBounds.GetFloat("right") / width;
			glyph.AtlasBounds.Top = atlasBounds.GetFloat("top") / height;
		}

		outFont.myGlyphs.emplace(static_cast<char>(glyph.UniCode), std::move(glyph));
	}

	return LoadTexture(aFontAtlasPath, outFont.myAtlas);
}

bool GraphicsEngine::LoadTexture(const std::filesystem::path& aPath, Texture& outTexture) const
{
	if (!std::filesystem::exists(aPath))
	{
		GELOG(Error, "Failed to load Texture {}! File not found!", aPath.string());
		return false;
	}

	std::ifstream file(aPath, std::ios::binary | std::ios::ate);

	if (!file)
	{
		GELOG(Error, "Failed to load Texture {}! Could not open file!", aPath.string());
		return false;
	}

	const std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> fileData(size);
	file.read(reinterpret_cast<char*>(fileData.data()), size);
	file.close();

	return myRHI->CreateTexture(aPath.stem().string(), fileData.data(), fileData.size(), outTexture);
}

void GraphicsEngine::SetEnvironmentTexture(std::unique_ptr<Texture> aEnvironmentTexture)
{
	myEnvironmentCubemap = std::move(aEnvironmentTexture);
}

bool GraphicsEngine::HasDrawCallFlag(DrawCallFlags aFlags, DrawCallFlag_ aFlag)
{
	return (aFlags & aFlag) != 0;
}

bool GraphicsEngine::CreateConstantBufferInternal(ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize)
{
	Buffer buffer;

	if (!myRHI->CreateConstantBuffer(aName, aBufferSize, buffer))
	{
		return false;
	}

	myConstantBuffers.emplace(aBufferID, std::move(buffer));

	return true;
}

bool GraphicsEngine::UpdateAndSetConstantBufferInternal(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const void* aData, size_t aDataSize, unsigned aSlot, PipelineStages aStages)
{
	if (!myConstantBuffers.contains(aBufferID))
	{
		GELOG(Warning, "Requested constant buffer update failed because this buffer does not exist!");
		return false;
	}

	const Buffer& buffer = myConstantBuffers.at(aBufferID);

	if (!inoutCommandList.UpdateConstantBuffer(buffer, aData, aDataSize))
	{
		return false;
	}

	inoutCommandList.SetConstantBuffer(&buffer, aSlot, aStages);

	return true;
}

bool GraphicsEngine::PrepareMeshForRendering(const Mesh& aMesh) const
{
	if (!aMesh.myVertexBuffer.IsValid())
	{
		if (!myRHI->CreateVertexBuffer(aMesh.myName, aMesh.myVertices, aMesh.myVertexBuffer))
		{
			GELOG(Warning, "Could not prepare mesh {}! Vertex Buffer could not be created!", aMesh.myName);
			return false;
		}
	}

	if (!aMesh.myIndexBuffer.IsValid())
	{
		if (!myRHI->CreateIndexBuffer(aMesh.myName, aMesh.myIndices, aMesh.myIndexBuffer))
		{
			GELOG(Warning, "Could not prepare mesh {}! Index Buffer could not be created!", aMesh.myName);
			return false;
		}
	}

	return true;
}

void GraphicsEngine::CreateMaterialTextureSlots(const RHIShaderReflectionInfo& aShaderInfo, Material& inoutMaterial) const
{
	for (const auto& shaderTextureSlot : aShaderInfo.Bindings)
	{
		if (shaderTextureSlot.Type != 2 || shaderTextureSlot.BindPoint >= Material::MAX_MATERIAL_TEXTURE_COUNT)
		{
			continue;
		}

		std::string lowerName = shaderTextureSlot.Name;
		std::ranges::transform(lowerName, lowerName.begin(), tolower);

		if (inoutMaterial.myTexturesSlotNameToIndex.contains(lowerName) && inoutMaterial.myTexturesSlotNameToIndex.at(lowerName) != shaderTextureSlot.BindPoint)
		{
			GELOG(Warning, "Found texture {} in multiple places when setting up material. Only the first instance will be used!", shaderTextureSlot.Name);
			continue;
		}

		inoutMaterial.myTexturesSlotNameToIndex.emplace(lowerName, shaderTextureSlot.BindPoint);
	}
}

void GraphicsEngine::RenderShadowMaps(GraphicsCommandList& inoutCommandList, const std::vector<SceneLight>& SceneLights, const SceneCamera& aSceneCamera, std::vector<InternalRenderItem>& aShadowItems)
{
	for (const auto& light : SceneLights)
	{
		switch (light.LightData->Type)
		{
			case LightType::Directional:
			{
				// TODO: Blend between cascades
				// https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps

				static float cameraNear = 0.0f;
				static float cameraFar = 0.0f;

				float cameraNearPlane = aSceneCamera.RenderCamera->GetNearPlane();
				float cameraFarPlane = aSceneCamera.RenderCamera->GetFarPlane();
				float cameraFov = aSceneCamera.RenderCamera->GetFovInDegrees();
				float cameraAspectRatio = aSceneCamera.RenderCamera->GetAspectRatio();

				if (cameraNear != cameraNearPlane || cameraFar != cameraFarPlane)
				{
					cameraNear = cameraNearPlane;
					cameraFar = cameraFarPlane;
					BuildShadowCascades({4096, 2048, 2048, 2048 }, cameraNear, cameraFar);
				}

				CommonUtilities::Matrix4x4f lightViewProjectionTextures[NUM_CASCADES];

				CommonUtilities::Matrix4x4f cameraView = aSceneCamera.Transform.GetFastInverse();

				for (size_t c = 0; c < NUM_CASCADES; c++ )
				{
					CommonUtilities::Frustum<float> frustum = CommonUtilities::Frustum<float>(
						cameraFov,
						myCascades[c].NearPlane,
						myCascades[c].FarPlane,
						cameraAspectRatio
					);

					CommonUtilities::Matrix4x4f projection = CommonUtilities::Matrix4x4f::CreatePerspectiveProjection(
						cameraFov,
						myCascades[c].NearPlane,
						myCascades[c].FarPlane,
						cameraAspectRatio
					);

					CommonUtilities::Matrix4x4f invViewProjection = (cameraView * projection).GetGeneralInverse();
					
					std::vector<CommonUtilities::Vector4f> corners = frustum.GetCornersInWorldSpace(invViewProjection);

					CommonUtilities::Vector4f center;

					for (auto& corner : corners)
					{
						center += corner;
					}

					center = center / 8.0f;

					float radius = 0.0f;

					for (const auto& corner : corners)
					{
						float distance = (CommonUtilities::Vector3f(corner.x, corner.y, corner.z) - center.ToXYZ()).Length();
						radius = std::max(radius, distance);
					}

					radius = std::ceil(radius);

					float texelsPerUnit = myCascades[c].ShadowMap->GetViewport().Width / (radius * 2.0f);

					CommonUtilities::Vector3f eye = center.ToXYZ() - light.Transform.GetForward().GetNormalized() * radius;

					CommonUtilities::Matrix4x4f directionLightTransform = light.Transform;
					directionLightTransform.SetTranslation(eye);
					CommonUtilities::Matrix4x4f lightView = directionLightTransform.GetFastInverse();

					CommonUtilities::Vector4f shadowOrigin = (CommonUtilities::Vector4f(0, 0, 0, 1) * lightView) * texelsPerUnit;
					CommonUtilities::Vector4f rounded = { std::round(shadowOrigin.x), std::round(shadowOrigin.y), shadowOrigin.z, shadowOrigin.w };
					CommonUtilities::Vector4f offset = (rounded - shadowOrigin) / texelsPerUnit;
					
					lightView(4, 1) += offset.x;
					lightView(4, 2) += offset.y;

					float orthoNear = -radius * 4.0f;
					float orthoFar = radius * 4.0f;

					CommonUtilities::Matrix4x4f orthoProjection =
						CommonUtilities::Matrix4x4f::CreateOrthographicProjection(orthoNear, orthoFar, -radius, radius, -radius, radius);

					lightViewProjectionTextures[c] = lightView * orthoProjection * CommonUtilities::Matrix4x4f::CreateNDCToTextureSpace();

					{
						FrameBuffer frameBuffer;
						frameBuffer.Projection = orthoProjection;
						frameBuffer.View = lightView;

						inoutCommandList.BeginEvent(std::format("Cascade Shadow Pass {}", (c + 1)));

						inoutCommandList.ClearDepthStencil(*myCascades[c].ShadowMap);
						inoutCommandList.SetRenderTarget(nullptr, myCascades[c].ShadowMap.get());

						inoutCommandList.ChangeActiveOverrides(PipelineStage_Rasterizer | PipelineStage_PixelShader);
						UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader);

						RenderDrawCalls(inoutCommandList, aShadowItems, DrawCallFlag_BindMaterial);

						inoutCommandList.EndEvent();
					}
				}

				CascadeBuffer cascadeBuffer;
				cascadeBuffer.FarPlanes = 
				{ 
					myCascades[0].FarPlane,
					myCascades[1].FarPlane, 
					myCascades[2].FarPlane, 
					myCascades[3].FarPlane 
				};
				cascadeBuffer.ShadowMapSizes = 
				{ 
					myCascades[0].ShadowMap->GetViewport().Width,
					myCascades[1].ShadowMap->GetViewport().Width,
					myCascades[2].ShadowMap->GetViewport().Width,
					myCascades[3].ShadowMap->GetViewport().Width
				};
				cascadeBuffer.CameraPosition = aSceneCamera.Transform.GetTranslation();
				memcpy_s(cascadeBuffer.LightViewProjectionTextures, sizeof(CommonUtilities::Matrix4x4f) * NUM_CASCADES, lightViewProjectionTextures, sizeof(lightViewProjectionTextures));

				UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::CascadeBuffer, cascadeBuffer, 4, PipelineStage_PixelShader);
				
				break;
			}
			case LightType::Spot:
			{
				if (light.ShadowMap == nullptr)
				{
					continue;
				}

				FrameBuffer frameBuffer;
				frameBuffer.Projection = light.Projection;
				frameBuffer.View = light.Transform.GetFastInverse();
				frameBuffer.CameraPosition = light.Transform.GetTranslation();

				inoutCommandList.BeginEvent("Spot Light Shadow Pass");

				inoutCommandList.ClearDepthStencil(*light.ShadowMap);
				inoutCommandList.SetRenderTarget(nullptr, light.ShadowMap);

				inoutCommandList.ChangeActiveOverrides(PipelineStage_Rasterizer | PipelineStage_PixelShader);

				UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader);

				RenderDrawCalls(inoutCommandList, aShadowItems, DrawCallFlag_BindMaterial);

				inoutCommandList.EndEvent();

				break;
			}
			case LightType::Point:
			{
				if (light.ShadowMap == nullptr)
				{
					continue;
				}

				FrameBuffer frameBuffer;
				frameBuffer.Projection = light.Projection;
				frameBuffer.View = light.Transform.GetFastInverse();
				frameBuffer.CameraPosition = light.Transform.GetTranslation();

				inoutCommandList.BeginEvent("Point Light Shadow Pass");

				inoutCommandList.ClearDepthStencil(*light.ShadowMap);
				inoutCommandList.SetRenderTarget(nullptr, light.ShadowMap);

				inoutCommandList.ChangeActiveOverrides(PipelineStage_GeometryShader | PipelineStage_Rasterizer | PipelineStage_PixelShader);

				UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_GeometryShader);

				RenderDrawCalls(inoutCommandList, aShadowItems, DrawCallFlag_BindMaterial);

				inoutCommandList.EndEvent();

				break;
			}
		}
	}
}

void GraphicsEngine::UploadSceneLightingData(GraphicsCommandList& inoutCommandList, const std::vector<SceneLight>& aLights)
{
	LightBuffer lightBuffer;
	lightBuffer.NumLights = static_cast<unsigned int>(std::min(aLights.size(), static_cast<size_t>(MAX_LIGHTS)));

	std::vector<const Texture*> dirLightShadowMaps;
	dirLightShadowMaps.reserve(NUM_CASCADES);
	std::vector<const Texture*> spotLightShadowMaps;
	std::vector<const Texture*> pointLightShadowCubes;

	unsigned int spotShadowIndex = 0;
	unsigned int pointShadowIndex = 0;

	for (unsigned int i = 0; i < lightBuffer.NumLights; i++)
	{
		if (aLights[i].LightData == nullptr)
		{
			continue;
		}

		lightBuffer.Lights[i] = *aLights[i].LightData;

		switch (aLights[i].LightData->Type)
		{
			case LightType::Directional:
			{
				for (size_t c = 0; c < NUM_CASCADES; c++)
				{
					dirLightShadowMaps.emplace_back(myCascades[c].ShadowMap.get());
				}

				break;
			}
			case LightType::Spot:
			{
				if (aLights[i].ShadowMap == nullptr)
				{
					continue;
				}

				spotLightShadowMaps.emplace_back(aLights[i].ShadowMap);
				lightBuffer.Lights[i].ShadowIndex = spotShadowIndex++;
				break;
			}
			case LightType::Point:
			{
				if (aLights[i].ShadowMap == nullptr)
				{
					continue;
				}

				pointLightShadowCubes.emplace_back(aLights[i].ShadowMap);
				lightBuffer.Lights[i].ShadowIndex = pointShadowIndex++;
				break;
			}
		}
	}

	inoutCommandList.SetShaderResources(dirLightShadowMaps.data(), dirLightShadowMaps.size(), 100, PipelineStage_PixelShader);
	inoutCommandList.SetShaderResources(spotLightShadowMaps.data(), spotLightShadowMaps.size(), 104, PipelineStage_PixelShader);
	inoutCommandList.SetShaderResources(pointLightShadowCubes.data(), pointLightShadowCubes.size(), 112, PipelineStage_PixelShader);

	UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::LightBuffer, lightBuffer, 3, PipelineStage_PixelShader);
}

void GraphicsEngine::RenderDrawCalls(GraphicsCommandList& inoutCommandList, const std::vector<InternalRenderItem>& aRenderItems, DrawCallFlags someFlags)
{
	for (const auto& renderItem : aRenderItems)
	{
		if (renderItem.DrawCall == nullptr)
		{
			GELOG(Error, "Tried to render a draw call with a null mesh!");
			continue;
		}

		const DrawCall& drawCall = *renderItem.DrawCall;
		const Mesh& mesh = *drawCall.Mesh;
		const Mesh::Element& element = renderItem.DrawCall->Mesh->myElements[renderItem.ElementIndex];
		const MaterialInterface* material = renderItem.DrawCall->Materials[renderItem.MaterialIndex];;

		if (!PrepareMeshForRendering(mesh))
		{
			continue;
		}

		inoutCommandList.BeginEvent(mesh.GetName());

		inoutCommandList.SetVertexBuffer(&mesh.myVertexBuffer);
		inoutCommandList.SetIndexBuffer(&mesh.myIndexBuffer);

		ObjectBuffer objectBuffer;
		objectBuffer.World = drawCall.Transform;
		objectBuffer.WorldInvT = (drawCall.Transform.GetInverse()).GetTranspose();
		objectBuffer.HasSkinning = drawCall.HasSkinning;

		UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader);

		if (drawCall.HasSkinning)
		{
			AnimationBuffer animationBuffer;
			memcpy(animationBuffer.JointTransforms, drawCall.JointTransforms, 128 * sizeof(CommonUtilities::Matrix4x4f));
			UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::AnimationBuffer, animationBuffer, 2, PipelineStage_VertexShader);
		}

		if (HasDrawCallFlag(someFlags, DrawCallFlag_BindMaterial))
		{
			if (material->HasParameters())
			{
				if (material->IsMaterialDataDirty())
				{
					material->RefreshMaterialData();
				}

				UpdateAndSetConstantBufferInternal(inoutCommandList, ConstantBuffer::MaterialBuffer, material->GetParameterDataBlock(), Material::MATERIAL_BUFFER_SIZE, 5, PipelineStage_VertexShader | PipelineStage_PixelShader);
			}

			if (HasDrawCallFlag(someFlags, DrawCallFlag_BindTextures))
			{
				std::vector<const Texture*> textures(Material::MAX_MATERIAL_TEXTURE_COUNT);

				for (size_t t = 0; t < Material::MAX_MATERIAL_TEXTURE_COUNT; t++)
				{
					const std::shared_ptr<Texture>& texture = material->GetTexture(static_cast<unsigned int>(t));

					if (texture != nullptr)
					{
						textures[t] = texture.get();
					}
				}

				inoutCommandList.SetShaderResources(textures.data(), textures.size(), 0, PipelineStage_VertexShader | PipelineStage_PixelShader);
			}

			inoutCommandList.SetPipelineStateObject(material->GetPSO());
		}
			
		inoutCommandList.DrawIndexed(element.NumIndices, element.IndexOffset);

		// Ends event for the draw call!
		inoutCommandList.EndEvent();
	}
}

void GraphicsEngine::RegisterMaterialShaders()
{
	// Domain
	{
		myMaterialDomainShaders.emplace(MaterialDomain::Surface, myShaderRoot / "Material" / "Surface_VS.hlsl");
	}

	// Shading Model
	{
		myMaterialShadingModelShaders.emplace(ShadingModel::Unlit, myShaderRoot / "Material" / "Unlit_PS.hlsl");
		myMaterialShadingModelShaders.emplace(ShadingModel::Lit, myShaderRoot / "Material" / "PBL_PS.hlsl");
	}
}

void GraphicsEngine::BuildShadowCascades(const std::array<int, NUM_CASCADES>& aCascadeSizes, float aCameraNear, float aCameraFar)
{
	assert(aCameraNear > 0.0f);
	assert(aCameraFar > aCameraNear);

	// Range from 0 to 1 to balance distributions
    float lambda = 0.55f;

    std::array<float, NUM_CASCADES + 1> splits = {};
	splits[0] = aCameraNear;

    for (int c = 1; c <= NUM_CASCADES; c++)
    {
		float cascadeRatio = static_cast<float>(c) / static_cast<float>(NUM_CASCADES);

		float linearSplit = aCameraNear + cascadeRatio * (aCameraFar - aCameraNear);
        float logSplit = aCameraNear * std::pow((aCameraFar / aCameraNear), cascadeRatio);

        splits[c] = lambda * logSplit + (1.0f - lambda) * linearSplit;
    }

	for (size_t c = 0; c < NUM_CASCADES; c++)
	{
		myCascades[c].NearPlane = splits[c];
		myCascades[c].FarPlane = splits[c + 1];

		std::string name = std::format("Shadow Cascade {}", (c + 1));

		myCascades[c].ShadowMap = std::make_unique<Texture>();

		CreateShadowMap(name, aCascadeSizes[c], aCascadeSizes[c], *myCascades[c].ShadowMap, false);
	}
}

bool GraphicsEngine::InitializeBuiltinAssets()
{
	// Material
	{
		myDefaultMaterial = std::make_unique<Material>();

		MaterialDescription defaultMatDesc;
		defaultMatDesc.Name = "Default";
		defaultMatDesc.BlendMode = MaterialBlendMode::Opaque;
		defaultMatDesc.Domain = MaterialDomain::Surface;
		defaultMatDesc.ShadingModel = ShadingModel::Unlit;
		defaultMatDesc.MaterialShaderCode = myShaderRoot / "Material" / "Include" / "Material.hlsli";

		if (!CreateMaterial(defaultMatDesc, *myDefaultMaterial))
		{
			GELOG(Error, "Failed to create default material!");
			return false;
		}
	}

	// Albedo Texture
	{
		myDefaultAlbedoTexture = std::make_shared<Texture>();

		if (!myRHI->CreateTexture("Default Albedo", BuiltIn_Default_C_ByteCode, sizeof(BuiltIn_Default_C_ByteCode), *myDefaultAlbedoTexture))
		{
			GELOG(Error, "Failed to create default albedo texture!");
			return false;
		}

		myDefaultMaterial->SetTexture(0, myDefaultAlbedoTexture);
	}

	// Normal Texture
	{
		myDefaultNormalTexture = std::make_shared<Texture>();

		if (!myRHI->CreateTexture("Default Normal", BuiltIn_Default_N_ByteCode, sizeof(BuiltIn_Default_N_ByteCode), *myDefaultNormalTexture))
		{
			GELOG(Error, "Failed to create default normal texture!");
			return false;
		}

		myDefaultMaterial->SetTexture(1, myDefaultNormalTexture);
	}

	// Font
	{
		myDefaultFont = std::make_shared<Font>();

		// Command - msdf-atlas-gen -varfont "FONTNAME.ttf?wght=400" -type msdf -size 64 -imageout FONTNAME.png -json FONTNAME.json -potr -pxrange 6
		// NOTE: msdf-atlas-gen can't output DDS, so convert the PNG afterwards.
		// Convert to DDS:      texconv FONTNAME.png -f R8G8B8A8_UNORM -m 1 -y

		std::filesystem::path defaultFontRoot = myContentRoot / "Fonts" / "CascadiaCode";

		if (!CreateFontAsset(defaultFontRoot / "cascadiacode.json", defaultFontRoot / "cascadiacode.dds", *myDefaultFont))
		{
			GELOG(Error, "Failed to create default font asset!");
			return false;
		}

		myDefaultFontMaterial = std::make_shared<Material>();

		MaterialDescription fontMaterialDesc;
		fontMaterialDesc.Name = "FontMaterial";
		fontMaterialDesc.Domain = MaterialDomain::Surface;
		fontMaterialDesc.ShadingModel = ShadingModel::Unlit;
		fontMaterialDesc.BlendMode = MaterialBlendMode::Alpha;
		fontMaterialDesc.PixelShader = myShaderRoot / "Material" / "FontMaterial_PS.hlsl";
		fontMaterialDesc.VertexShader = myShaderRoot / "Material" / "FontMaterial_VS.hlsl";

		if (!CreateMaterial(fontMaterialDesc, *myDefaultFontMaterial))
		{
			GELOG(Error, "Failed to create font material!");
			return false;
		}

		myText = std::make_unique<Text>("Text Test");

		myText->SetFont(myDefaultFont);
		myText->SetSize(36);
		myText->SetPosition({ 100, 50 });
		myText->SetColor({ 0, 1, 0, 1 });
		myText->SetText("Shrek");
		myText->SetFontMaterial(myDefaultFontMaterial);
	}

	return true;
}

bool GraphicsEngine::InitializeRenderResources()
{
	// Constant Buffers
	{
		if (!CreateConstantBuffer<FrameBuffer>(ConstantBuffer::FrameBuffer, "FrameBuffer"))
		{
			return false;
		}
		if (!CreateConstantBuffer<ObjectBuffer>(ConstantBuffer::ObjectBuffer, "ObjectBuffer"))
		{
			return false;
		}
		if (!CreateConstantBuffer<AnimationBuffer>(ConstantBuffer::AnimationBuffer, "AnimationBuffer"))
		{
			return false;
		}
		if (!CreateConstantBuffer<LightBuffer>(ConstantBuffer::LightBuffer, "LightBuffer"))
		{
			return false;
		}
		if (!CreateConstantBuffer(ConstantBuffer::MaterialBuffer, "MaterialBuffer", Material::MATERIAL_BUFFER_SIZE))
		{
			return false;
		}
		if (!CreateConstantBuffer<CascadeBuffer>(ConstantBuffer::CascadeBuffer, "CascadeBuffer"))
		{
			return false;
		}
	}

	// Trilinear Wrap
	{
		SamplerDescription samplerDesc;
		samplerDesc.Name = "TrilinearWrap";
		samplerDesc.AddressMode = SamplerAddressMode::Wrap;
		samplerDesc.FilterMode = SamplerFilterMode::Trilinear;

		std::unique_ptr<Sampler> sampler = std::make_unique<Sampler>();

		if (!myRHI->CreateSampler(samplerDesc, *sampler))
		{
			return false;
		}

		mySamplers.emplace_back(std::move(sampler));
	}

	// Shadow Sampler
	{
		SamplerDescription samplerDesc;
		samplerDesc.Name = "ShadowSampler";
		samplerDesc.AddressMode = SamplerAddressMode::Border;
		samplerDesc.FilterMode = SamplerFilterMode::ComparisonLinearPoint;
		samplerDesc.ComparisonFunction = SamplerComparisonFunc::LessEqual;
		samplerDesc.BorderColor = CommonUtilities::Vector4f::One;

		std::unique_ptr<Sampler> sampler = std::make_unique<Sampler>();

		if (!myRHI->CreateSampler(samplerDesc, *sampler))
		{
			return false;
		}

		mySamplers.emplace_back(std::move(sampler));
	}

	// LUT Sampler
	{
		SamplerDescription samplerDesc;
		samplerDesc.Name = "LUTSampler";
		samplerDesc.AddressMode = SamplerAddressMode::Clamp;
		samplerDesc.FilterMode = SamplerFilterMode::Trilinear;
		samplerDesc.BorderColor = CommonUtilities::Vector4f::One;

		std::unique_ptr<Sampler> sampler = std::make_unique<Sampler>();

		if (!myRHI->CreateSampler(samplerDesc, *sampler))
		{
			return false;
		}

		mySamplers.emplace_back(std::move(sampler));
	}

	// Trilinear Clamp
	{
		SamplerDescription samplerDesc;
		samplerDesc.Name = "TrilinearClamp";
		samplerDesc.AddressMode = SamplerAddressMode::Clamp;
		samplerDesc.FilterMode = SamplerFilterMode::Trilinear;

		std::unique_ptr<Sampler> sampler = std::make_unique<Sampler>();

		if (!myRHI->CreateSampler(samplerDesc, *sampler))
		{
			return false;
		}

		mySamplers.emplace_back(std::move(sampler));
	}

	// Shadow PSO
	{
		PipelineStateDescription shadowPSODesc = {};
		shadowPSODesc.Name = "ShadowPSO";
		shadowPSODesc.RasterizerState = RasterizerState::FillSolidCullFront;
		shadowPSODesc.GeometryShader.ByteCode = INTERNAL_ShadowCube_GS_ByteCode;
		shadowPSODesc.GeometryShader.ByteCodeSize = sizeof(INTERNAL_ShadowCube_GS_ByteCode);

		myShadowPSO = std::make_unique<PipelineStateObject>();

		if (!myRHI->CreatePipelineStateObject(shadowPSODesc, *myShadowPSO))
		{
			GELOG(Error, "Failed to create Shadow pipeline state object!");
			return false;
		}
	}

	// GBuffer & Gbuffer PSO
	{
		PipelineStateDescription gBufferPSODesc = {};
		gBufferPSODesc.Name = "GBufferPSO";
		gBufferPSODesc.BlendMode = BlendMode::Opaque;
		gBufferPSODesc.VertexShader.ByteCode = INTERNAL_Surface_VS_ByteCode;
		gBufferPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Surface_VS_ByteCode);
		gBufferPSODesc.PixelShader.ByteCode = INTERNAL_GBuffer_PS_ByteCode;
		gBufferPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_GBuffer_PS_ByteCode);

		myGBufferPSO = std::make_unique<PipelineStateObject>();

		if (!myRHI->CreatePipelineStateObject(gBufferPSODesc, *myGBufferPSO))
		{
			GELOG(Error, "Failed to create GBuffer pipeline state object!");
			return false;
		}

		myGBuffer = std::make_unique<GBuffer>();

		CommonUtilities::Vector2ui renderSize = myRHI->GetClientSize();

		myRHI->CreateGBuffer("GBuffer", renderSize.x, renderSize.y, *myGBuffer);
	}

	// LUT Texture
	{
		PipelineStateDescription lutPSODesc = {};
		lutPSODesc.Name = "LUTPSO";
		lutPSODesc.Topology = Topology::TriangleStrip;
		lutPSODesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		lutPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		lutPSODesc.PixelShader.ByteCode = INTERNAL_BRDF_LUT_PS_ByteCode;
		lutPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_BRDF_LUT_PS_ByteCode);

		PipelineStateObject lutPSO;

		if (!myRHI->CreatePipelineStateObject(lutPSODesc, lutPSO))
		{
			GELOG(Error, "Failed to create LUT pipeline state object!");
			return false;
		}

		myLUTTexture = std::make_unique<Texture>();

		if (!myRHI->CreateTexture("BRDF_LUT", 512, 512, 1, ResourceFormat::R16G16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myLUTTexture))
		{
			GELOG(Error, "Failed to create BRDF LUT texture!");
			return false;
		}

		GraphicsCommandList initCommandList;

		if (!myRHI->CreateCommandList("LUT Init", initCommandList))
		{
			GELOG(Error, "Failed to create LUT init command list!");
			return false;
		}

		initCommandList.SetPipelineStateObject(lutPSO);
		initCommandList.SetRenderTarget(myLUTTexture.get(), nullptr);
		initCommandList.Draw(4);
		initCommandList.FinishCommandList();

		myRHI->ExecuteCommandList(initCommandList);
	}

	// Deffered Ambient light
	{
		PipelineStateDescription deferredDirLightDesc;
		deferredDirLightDesc.Name = "DeferredAmbientLight";
		deferredDirLightDesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		deferredDirLightDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		deferredDirLightDesc.PixelShader.ByteCode = INTERNAL_Deferred_Ambient_PS_ByteCode;
		deferredDirLightDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Deferred_Ambient_PS_ByteCode);
		deferredDirLightDesc.BlendMode = BlendMode::Additive;
		deferredDirLightDesc.Topology = Topology::TriangleStrip;

		myDeferredAmbientPSO = std::make_unique<PipelineStateObject>();

		if (!myRHI->CreatePipelineStateObject(deferredDirLightDesc, *myDeferredAmbientPSO))
		{
			GELOG(Error, "Failed to create deferred ambient light pipeline state object!");
			return false;
		}
	}

	// Deffered Dir Light PSO
	{
		PipelineStateDescription deferredDirLightDesc;
		deferredDirLightDesc.Name = "DeferredDirLight";
		deferredDirLightDesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		deferredDirLightDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		deferredDirLightDesc.PixelShader.ByteCode = INTERNAL_Deferred_DirLight_PS_ByteCode;
		deferredDirLightDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Deferred_DirLight_PS_ByteCode);
		deferredDirLightDesc.BlendMode = BlendMode::Additive; 
		deferredDirLightDesc.Topology = Topology::TriangleStrip;

		myDeferredDirPSO = std::make_unique<PipelineStateObject>();

		if (!myRHI->CreatePipelineStateObject(deferredDirLightDesc, *myDeferredDirPSO))
		{
			GELOG(Error, "Failed to create deferred directional light pipeline state object!");
			return false;
		}
	}

	// Deffered Spot Light PSO
	{
		PipelineStateDescription deferredSpotLightDesc;
		deferredSpotLightDesc.Name = "DeferredSpotLight";
		deferredSpotLightDesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		deferredSpotLightDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		deferredSpotLightDesc.PixelShader.ByteCode = INTERNAL_Deferred_SpotLight_PS_ByteCode;
		deferredSpotLightDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Deferred_SpotLight_PS_ByteCode);
		deferredSpotLightDesc.BlendMode = BlendMode::Additive;
		deferredSpotLightDesc.Topology = Topology::TriangleStrip;

		myDeferredSpotPSO = std::make_unique<PipelineStateObject>();

		if (!myRHI->CreatePipelineStateObject(deferredSpotLightDesc, *myDeferredSpotPSO))
		{
			GELOG(Error, "Failed to create deferred spot light pipeline state object!");
			return false;
		}
	}

	// Deffered Point Light PSO
	{
		PipelineStateDescription deferredPointLightDesc;
		deferredPointLightDesc.Name = "DeferredPointLight";
		deferredPointLightDesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		deferredPointLightDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		deferredPointLightDesc.PixelShader.ByteCode = INTERNAL_Deferred_PointLight_PS_ByteCode;
		deferredPointLightDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Deferred_PointLight_PS_ByteCode);
		deferredPointLightDesc.BlendMode = BlendMode::Additive;
		deferredPointLightDesc.Topology = Topology::TriangleStrip;

		myDeferredPointPSO = std::make_unique<PipelineStateObject>();

		if (!myRHI->CreatePipelineStateObject(deferredPointLightDesc, *myDeferredPointPSO))
		{
			GELOG(Error, "Failed to create deferred point light pipeline state object!");
			return false;
		}
	}

	return true;
}