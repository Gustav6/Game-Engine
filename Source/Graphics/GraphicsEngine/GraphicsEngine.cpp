#include "GraphicsEngine.pch.h"
#include "GraphicsEngine.hpp"

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Vector4.hpp"

#include "Objects/Mesh/Mesh.h"

#include "Objects/Scene/Light.h"

#include "Objects/GPU/Texture.h"
#include "Objects/GPU/Shader.h"
#include "Objects/GPU/Sampler.h"
#include "Objects/GPU/GBuffer.h"

#include "Objects/Text/Text.h"
#include "Objects/Sprite.h"

#include "Resources/BuiltinAssets.h"
#include "Resources/GraphicsResourceFactory.h"

#include "ConstantBuffers/FrameBuffer.h"
#include "ConstantBuffers/ObjectBuffer.h"
#include "ConstantBuffers/AnimationBuffer.h"
#include "ConstantBuffers/LightBuffer.h"
#include "ConstantBuffers/CascadeBuffer.h"
#include <ConstantBuffers/Manager/ConstantBufferManager.h>

#include "RHI/GraphicsCommandList.h"
#include "RHI/PipelineStateObject.h"
#include "RHI/RenderHardwareInterface.h"

#include "Materials/Material.h"
#include "Materials/MaterialHelpers.h"
#include "Materials/MaterialShaderIncludeHandler.h"

#include "Materials/Compilation/MaterialFactory.h"
#include "Materials/Compilation/MaterialShaderRegistry.h"

#include "InternalPasses/DeferredRenderPass.h"
#include "InternalPasses/ForwardRenderPass.h"
#include "InternalPasses/ShadowRenderPass.h"
#include "InternalPasses/ScreenSpaceRenderPass.h"

#include "Core/RenderItem.h"
#include "Core/FrameTargets.h"
#include "Core/RenderContext.h"
#include "Core/ConstantBufferID.h"

#include "FullTexture_VS.h"
#include "BRDF_LUT_PS.h"

#include "Drawers/MeshDrawer.h"
#include "Drawers/LightDrawer.h"
#include "Drawers/TextDrawer.h"
#include "Drawers/SpriteDrawer.h"

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

	myMaterialShaderRegistry = std::make_unique<MaterialShaderRegistry>();
	myMaterialShaderRegistry->Initialize(myShaderRoot);

	myBackBuffer = std::make_unique<Texture>();
	myDepthBuffer = std::make_unique<Texture>();

	myRHI = std::make_unique<RenderHardwareInterface>();

	if (!myRHI->Initialize(aWindowHandle, true, *myBackBuffer, *myDepthBuffer))
	{
		return false;
	}

	myBuiltinAssets = std::make_unique<BuiltinAssets>();

	if (!myBuiltinAssets->Initialize(*this, *myRHI, myShaderRoot))
	{
		return false;
	};

	myConstantBufferManager = std::make_unique<ConstantBufferManager>();

	if (!InitializeRenderResources())
	{
		return false;
	};

	// Init passes
	{
		myForwardRenderPass = std::make_unique<ForwardRenderPass>();
		myDeferredRenderPass = std::make_unique<DeferredRenderPass>();
		myScreenSpaceRenderPass = std::make_unique<ScreenSpaceRenderPass>(*myRHI);
		myShadowRenderPass = std::make_unique<ShadowRenderPass>();

		myDeferredRenderPass->Initialize(*myRHI);
		myShadowRenderPass->Initialize(*myRHI);
	}

	return true;
}

void GraphicsEngine::BeginFrame()
{
	MeshDrawer::Get().myDrawCalls.clear();
	LightDrawer::Get().mySceneLights.clear();
	TextDrawer::Get().myDrawCalls.clear();
	SpriteDrawer::Get().myDrawCalls.clear();
}

void GraphicsEngine::Render(GraphicsCommandList& inoutCommandList, const SceneCamera& aCamera)
{
	{
		inoutCommandList.BeginEvent("Clear Backbuffer/Depth");

		inoutCommandList.ClearRenderTarget(*myBackBuffer);
		inoutCommandList.ClearDepthStencil(*myDepthBuffer);

		inoutCommandList.EndEvent();
	}

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
		[cameraPosition](const RenderItem& A, const RenderItem& B) -> bool
		{
			float distanceToA = (A.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();
			float distanceToB = (B.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();

			return distanceToA < distanceToB;
		});

	// Back to Front
	std::sort(alphaItems.begin(), alphaItems.end(),
		[cameraPosition](const RenderItem& A, const RenderItem& B) -> bool
		{
			float distanceToA = (A.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();
			float distanceToB = (B.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();

			return distanceToA > distanceToB;
		});

	std::vector<ScreenSpaceItem> screenSpaceItems;
	screenSpaceItems.reserve(SpriteDrawer::Get().myDrawCalls.size() + TextDrawer::Get().myDrawCalls.size());

	for (const auto& drawCall : SpriteDrawer::Get().myDrawCalls)
	{
		ScreenSpaceItem item =
		{
			.SpriteCall = &drawCall,
			.Depth = drawCall.SpriteInstanceData->Position.z
		};

		screenSpaceItems.push_back(item);
	}

	for (const auto& drawCall : TextDrawer::Get().myDrawCalls)
	{
		ScreenSpaceItem item =
		{
			.TextCall = &drawCall,
			.Depth = drawCall.Text->myZOrder
		};

		screenSpaceItems.push_back(item);
	}

	std::sort(screenSpaceItems.begin(), screenSpaceItems.end(),
		[](const ScreenSpaceItem& A, const ScreenSpaceItem& B)
		{
			return A.Depth > B.Depth;
		});

	RenderContext context =
	{
		.ConstantBuffers = myConstantBufferManager.get(),
		.RHI = myRHI.get(),
		.BuiltinAssets = myBuiltinAssets.get(),
		.Camera = &aCamera,
		.Mode = myDebugRenderMode,
		.CascadeShadows = &myShadowRenderPass->GetCascades()
	};

	myShadowRenderPass->Execute(inoutCommandList, shadowItems, context);

	inoutCommandList.BeginEvent("Main Pass");
	{
		FrameTargets targets =
		{
			.BackBuffer = myBackBuffer.get(),
			.DepthBuffer = myDepthBuffer.get()
		};

		myDeferredRenderPass->Execute(inoutCommandList, opaqueItems, targets, context);
		myForwardRenderPass->Execute(inoutCommandList, alphaItems, targets, context);
		myScreenSpaceRenderPass->Execute(inoutCommandList, screenSpaceItems, targets, context);
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
	return MaterialFactory::Create(aDescription, *myRHI, *myMaterialShaderRegistry, myShaderRoot, outMaterial);
}

bool GraphicsEngine::CreateShadowMap(std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outShadowMap, bool aCube) const
{
	return GraphicsResourceFactory::CreateShadowMap(*myRHI, aName, aWidth, aHeight, outShadowMap, aCube);
}

bool GraphicsEngine::CreateTexture(std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture) const
{
	return GraphicsResourceFactory::CreateTexture(*myRHI, aName, aWidth, aHeight, aNumMips, aFormat, aBindFlags, outTexture);
}

bool GraphicsEngine::CreateTexture(const std::filesystem::path& aPath, Texture& outTexture) const
{
	return GraphicsResourceFactory::CreateTexture(*myRHI, aPath, outTexture);
}

bool GraphicsEngine::CreateFontAsset(const std::filesystem::path& aFontPath, const std::filesystem::path& aFontAtlasPath, Font& outFont) const
{
	return GraphicsResourceFactory::CreateFontAsset(*myRHI, aFontPath, aFontAtlasPath, outFont);
}

void GraphicsEngine::SetEnvironmentTexture(std::shared_ptr<Texture> aEnvironmentTexture)
{
	myEnvironmentCubemap = std::move(aEnvironmentTexture);
}

bool GraphicsEngine::InitializeRenderResources()
{
	// Constant Buffers
	{
		if (!myConstantBufferManager->CreateConstantBuffer<FrameBuffer>(*myRHI, ConstantBuffer::FrameBuffer, "FrameBuffer"))
		{
			return false;
		}
		if (!myConstantBufferManager->CreateConstantBuffer<ObjectBuffer>(*myRHI, ConstantBuffer::ObjectBuffer, "ObjectBuffer"))
		{
			return false;
		}
		if (!myConstantBufferManager->CreateConstantBuffer<AnimationBuffer>(*myRHI, ConstantBuffer::AnimationBuffer, "AnimationBuffer"))
		{
			return false;
		}
		if (!myConstantBufferManager->CreateConstantBuffer<LightBuffer>(*myRHI, ConstantBuffer::LightBuffer, "LightBuffer"))
		{
			return false;
		}
		if (!myConstantBufferManager->CreateConstantBuffer<CascadeBuffer>(*myRHI, ConstantBuffer::CascadeBuffer, "CascadeBuffer"))
		{
			return false;
		}
		if (!myConstantBufferManager->CreateConstantBuffer(*myRHI, ConstantBuffer::MaterialBuffer, "MaterialBuffer", Material::MATERIAL_BUFFER_SIZE))
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