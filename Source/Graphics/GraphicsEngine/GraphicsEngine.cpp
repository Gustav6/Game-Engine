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

#include <Assets/BuiltinAssets.h>

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

#include "RenderPasses/AlphaRenderPass.h"
#include "RenderPasses/OpaqueRenderPass.h"
#include "RenderPasses/ShadowRenderPass.h"
#include "RenderPasses/ScreenSpaceRenderPass.h"
#include "RenderPasses/Shared/RenderItem.h"
#include "RenderPasses/Shared/FrameTargets.h"
#include "RenderPasses/Shared/RenderContext.h"

#include "FullTexture_VS.h"
#include "BRDF_LUT_PS.h"

#include "Drawers/MeshDrawer.h"
#include "Drawers/LightDrawer.h"
#include "Drawers/TextDrawer.h"

#include "CommonUtilities/Json/JsonWrapper.h"

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

	{
		myMaterialDomainShaders.emplace(MaterialDomain::Surface, myShaderRoot / "Material" / "Surface_VS.hlsl");

		myMaterialShadingModelShaders.emplace(ShadingModel::Unlit, myShaderRoot / "Material" / "Unlit_PS.hlsl");
		myMaterialShadingModelShaders.emplace(ShadingModel::Lit, myShaderRoot / "Material" / "PBL_PS.hlsl");
	}

	myBuiltinAssets = std::make_unique<BuiltinAssets>();

	if (!myBuiltinAssets->Initialize(*this, *myRHI, myShaderRoot))
	{
		return false;
	};

	if (!InitializeRenderResources())
	{
		return false;
	};

	// Init passes
	{
		myAlphaRenderPass = std::make_unique<AlphaRenderPass>();
		myOpaqueRenderPass = std::make_unique<OpaqueRenderPass>();
		myScreenSpaceRenderPass = std::make_unique<ScreenSpaceRenderPass>();
		myShadowRenderPass = std::make_unique<ShadowRenderPass>();

		myOpaqueRenderPass->Initialize(*myRHI);
		myShadowRenderPass->Initialize(*myRHI);
	}

	return true;
}

void GraphicsEngine::BeginFrame()
{
	MeshDrawer::Get().myDrawCalls.clear();
	LightDrawer::Get().mySceneLights.clear();
	TextDrawer::Get().myDrawCalls.clear();
}

void GraphicsEngine::Render(GraphicsCommandList& inoutCommandList, const SceneCamera& aCamera)
{
	{
		inoutCommandList.BeginEvent("Reset Render Targets");

		inoutCommandList.ClearRenderTarget(*myBackBuffer);
		inoutCommandList.ClearDepthStencil(*myDepthBuffer);

		inoutCommandList.EndEvent();
	}

	std::vector<RenderItem> opaqueItems;
	std::vector<RenderItem> alphaItems;
	std::vector<RenderItem> shadowItems;

	for (const auto& drawCall : MeshDrawer::Get().myDrawCalls)
	{
		for (unsigned int e = 0; e < drawCall.Mesh->myElements.size(); e++)
		{
			unsigned int materialIndex = drawCall.Mesh->myElements[e].MaterialIndex;

			const MaterialInterface* material = drawCall.Materials[materialIndex];

			RenderItem item =
			{
				.DrawCall = &drawCall,
				.ElementIndex = e,
				.MaterialIndex = materialIndex
			};

			if (material == nullptr)
			{
				opaqueItems.push_back(item);
				shadowItems.push_back(item);
				continue;
			}

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

	CommonUtilities::Vector3f cameraPosition = aCamera.Transform.GetTranslation();

	// Front to Back
	std::sort(opaqueItems.begin(), opaqueItems.end(),
		[&](const RenderItem& A, const RenderItem& B) -> bool
		{
			float distanceToA = (A.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();
			float distanceToB = (B.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();

			return distanceToA < distanceToB;
		});

	// Back to Front
	std::sort(alphaItems.begin(), alphaItems.end(),
		[&](const RenderItem& A, const RenderItem& B) -> bool
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

	RenderContext context =
	{
		.RHI = myRHI.get(),
		.BuiltinAssets = myBuiltinAssets.get(),
		.Camera = &aCamera,
		.Mode = myDebugRenderMode,
		.CascadeShadows = myShadowRenderPass->GetCascades()
	};

	myShadowRenderPass->Execute(*this, inoutCommandList, shadowItems, context);

	inoutCommandList.BeginEvent("Main Pass");
	{
		FrameTargets targets =
		{
			.BackBuffer = myBackBuffer.get(),
			.DepthBuffer = myDepthBuffer.get()
		};

		myOpaqueRenderPass->Execute(*this, inoutCommandList, opaqueItems, targets, context);
		myAlphaRenderPass->Execute(*this, inoutCommandList, alphaItems, targets, context);
		myScreenSpaceRenderPass->Execute(*this, inoutCommandList, targets, context);
	}
	inoutCommandList.EndEvent();
}

void GraphicsEngine::Present() const
{
	myRHI->Present();
}

void GraphicsEngine::CycleToNextRenderPass(int aDirection)
{
	int nextDebugRenderMode = static_cast<int>(myDebugRenderMode) + aDirection;

	if (nextDebugRenderMode >= static_cast<int>(DebugRenderMode::MAX))
	{
		nextDebugRenderMode = 0;
	}
	else if (nextDebugRenderMode < 0)
	{
		nextDebugRenderMode = static_cast<int>(DebugRenderMode::MAX) - 1;
	}

	myDebugRenderMode = static_cast<DebugRenderMode>(nextDebugRenderMode);
}

bool GraphicsEngine::CreateConstantBuffer(ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize)
{
	Buffer buffer;

	if (!myRHI->CreateConstantBuffer(aName, aBufferSize, buffer))
	{
		return false;
	}

	myConstantBuffers.emplace(aBufferID, std::move(buffer));

	return true;
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

	return CreateTexture(aFontAtlasPath, outFont.myAtlas);
}

bool GraphicsEngine::CreateTexture(const std::filesystem::path& aPath, Texture& outTexture) const
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

bool GraphicsEngine::UpdateAndSetConstantBuffer(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const void* aData, size_t aDataSize, unsigned aSlot, PipelineStages aStages)
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

	return true;
}