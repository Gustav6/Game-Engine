#include "GraphicsEngine.pch.h"
#include "GraphicsEngine.hpp"

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Vector4.hpp"
#include "CommonUtilities/Collision/Intersection.hpp"

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

#include <ConstantBuffers/Manager/ConstantBufferManager.h>

#include "RHI/GraphicsCommandList.h"
#include "RHI/PipelineStateObject.h"
#include "RHI/RenderHardwareInterface.h"

#include "Materials/Material.h"
#include "Materials/MaterialShaderIncludeHandler.h"

#include "Materials/Compilation/MaterialFactory.h"
#include "Materials/Compilation/MaterialShaderRegistry.h"

#include "InternalPasses/DeferredRenderPass.h"
#include "InternalPasses/ForwardRenderPass.h"
#include "InternalPasses/ShadowRenderPass.h"
#include "InternalPasses/ScreenSpaceRenderPass.h"
#include "InternalPasses/ScreenSpaceEffectPass.h"

#include "Core/RenderItem.h"
#include "Core/FrameTargets.h"
#include "Core/RenderContext.h"
#include "Core/RenderEnums.h"
#include "Core/SceneCamera.h"
#include "Core/RenderOverlay.h"

#include "Drawers/MeshDrawer.h"
#include "Drawers/LightDrawer.h"
#include "Drawers/TextDrawer.h"
#include "Drawers/SpriteDrawer.h"
#include "Drawers/DebugDrawer.h"

#include "Fullscreen_VS.h"
#include "BRDF_LUT_PS.h"

#include <tracy/core/Tracy.hpp>

#include "InternalPasses/DebugRenderPass.h"

#include "Objects/Scene/Camera.h"

GraphicsEngine::GraphicsEngine() = default;
GraphicsEngine::~GraphicsEngine() = default;

GraphicsEngine& GraphicsEngine::Get()
{
	static GraphicsEngine myInstance;
	return myInstance;
}

bool GraphicsEngine::Initialize(HWND aWindowHandle, const std::filesystem::path& aContentRoot)
{
	if (!std::filesystem::exists(aContentRoot))
	{
		spdlog::critical("Content root dir is not a valid path! Provided root was {}.", aContentRoot.string());
		return false;
	}

	myShaderRoot = aContentRoot / "Shaders";
	
	if (!std::filesystem::exists(myShaderRoot))
	{
		spdlog::critical("Shader root dir is not a valid path! Provided root was {}.", myShaderRoot.string());
		return false;
	}

	myMaterialShaderRegistry = std::make_unique<MaterialShaderRegistry>();
	myMaterialShaderRegistry->Initialize(myShaderRoot);

	myBackBuffer = std::make_unique<Texture>();
	myDepthBuffer = std::make_unique<Texture>();

	myRHI = std::make_unique<RenderHardwareInterface>();

	if (!myRHI->Initialize(aWindowHandle, true, *myBackBuffer, *myDepthBuffer, myRenderSettings.VSyncEnabled))
	{
		return false;
	}

	myBuiltinAssets = std::make_unique<BuiltinAssets>();

	if (!myBuiltinAssets->Initialize(*this, *myRHI))
	{
		return false;
	}

	myConstantBufferManager = std::make_unique<ConstantBufferManager>();

	if (!myConstantBufferManager->Initialize(*myRHI))
	{
		return false;
	}

	// Init passes
	{
		myForwardRenderPass = std::make_unique<ForwardRenderPass>();
		myDeferredRenderPass = std::make_unique<DeferredRenderPass>();
		myScreenSpaceRenderPass = std::make_unique<ScreenSpaceRenderPass>();
		myShadowRenderPass = std::make_unique<ShadowRenderPass>();
		myScreenSpaceEffectPass = std::make_unique<ScreenSpaceEffectPass>();
		myDebugRenderPass = std::make_unique<DebugRenderPass>();

		if (!myDeferredRenderPass->Initialize(*myRHI))
		{
			return false;
		}
		if (!myShadowRenderPass->Initialize(*myRHI))
		{
			return false;
		}
		if (!myScreenSpaceRenderPass->Initialize(*myRHI))
		{
			return false;
		}
		if (!myScreenSpaceEffectPass->Initialize(*myRHI))
		{
			return false;
		}
		if (!myDebugRenderPass->Initialize(*myRHI))
		{
			return false;
		}
	}

	if (!InitializeRenderResources())
	{
		return false;
	}

	myRenderSettings.DebugRenderMode = DebugRenderMode::None;
	myRenderSettings.ToneMapper = ToneMapper::Unreal;
	myRenderOverlay = nullptr;
	myRenderSettings.BloomEnabled = true;
	myScreenSpaceEffectPass->SetBloomStatus(myRenderSettings.BloomEnabled);

	return true;
}

void GraphicsEngine::BeginFrame() const
{
	MeshDrawer::Get().Clear();
	LightDrawer::Get().Clear();
	TextDrawer::Get().Clear();
	SpriteDrawer::Get().Clear();
	DebugDrawer::Get().Clear();

	if (myRenderOverlay != nullptr)
	{
		myRenderOverlay->BeginFrame();
	}
}

FrameStats GraphicsEngine::Render(GraphicsCommandList& inoutCommandList, const SceneCamera& aCamera)
{
	ZoneScopedN("Whole render pass");

	FrameStats frameStats;

	{
		inoutCommandList.BeginEvent("Clear Backbuffers");

		inoutCommandList.ClearRenderTarget(*myBackBuffer);
		inoutCommandList.ClearRenderTarget(*myHDRBuffer);
		inoutCommandList.ClearDepthStencil(*myDepthBuffer);

		inoutCommandList.EndEvent();
	}

	{
		inoutCommandList.SetShaderSamplers(mySamplerPointers.data(), mySamplerPointers.size(), 0, PipelineStage_VertexShader | PipelineStage_PixelShader);
	}

	{
		inoutCommandList.SetShaderResource(myEnvironmentCubemap.get(), 98, PipelineStage_PixelShader);
		inoutCommandList.SetShaderResource(myLUTTexture.get(), 99, PipelineStage_PixelShader);
	}

	myDeferredItems.clear();
	myForwardItems.clear();

	for (const auto& drawCall : MeshDrawer::Get().GetMeshDrawCalls())
	{
		const CommonUtilities::Sphere<float>& boundingSphere = drawCall.Mesh->GetBoundingSphere();
		const CommonUtilities::Matrix4x4f& transform = drawCall.Transform;

		const CommonUtilities::Vector3f scale = transform.GetScaling();
		const float maxScale = std::max({ scale.x, scale.y, scale.z });

		const CommonUtilities::Vector4f transformedCenter = boundingSphere.GetCenter().ToXYZ1() * transform;

		const CommonUtilities::Sphere<float> worldSphere(transformedCenter.ToXYZ(),boundingSphere.GetRadius() * maxScale);

		if (!CommonUtilities::IntersectionSphereFrustum(worldSphere, aCamera.RenderCamera.GetFrustum()))
		{
			frameStats.CulledObjects++;
			continue;
		}

		frameStats.VisibleObjects++;

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
				myForwardItems.push_back(item);
			}
			else
			{
				switch (material->GetBlendMode())
				{
					case MaterialBlendMode::None:
					{
						spdlog::error("Mesh {} has a invalid Material Blend Mode setting!", drawCall.Mesh->GetName());
						break;
					}
					case MaterialBlendMode::Opaque:
					{
						myDeferredItems.push_back(item);
						break;
					}
					case MaterialBlendMode::Alpha:
					{
						myForwardItems.push_back(item);
						break;
					}
					default:
					{
						spdlog::error("Mesh {} has a invalid Material Blend Mode setting!", drawCall.Mesh->GetName());
						break;
					}
				}
			}
		}
	}

	myShadowItems.clear();
	myShadowItems.reserve(myDeferredItems.size());

	myShadowItems.insert(
		myShadowItems.end(),
		myDeferredItems.begin(),
		myDeferredItems.end()
	);

	CommonUtilities::Vector3f cameraPosition = aCamera.Transform.GetTranslation();

	// Front to Back
	std::sort(myDeferredItems.begin(), myDeferredItems.end(),
		[cameraPosition](const RenderItem& A, const RenderItem& B) -> bool
		{
			float distanceToA = (A.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();
			float distanceToB = (B.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();

			return distanceToA < distanceToB;
		});

	// Back to Front
	std::sort(myForwardItems.begin(), myForwardItems.end(),
		[cameraPosition](const RenderItem& A, const RenderItem& B) -> bool
		{
			float distanceToA = (A.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();
			float distanceToB = (B.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();

			return distanceToA > distanceToB;
		});

	std::vector<ScreenSpaceItem> screenSpaceItems;
	screenSpaceItems.reserve(SpriteDrawer::Get().GetSpriteDrawCalls().size() + TextDrawer::Get().GetTextDrawCalls().size());

	for (const auto& drawCall : SpriteDrawer::Get().GetSpriteDrawCalls())
	{
		ScreenSpaceItem item =
		{
			.SpriteCall = &drawCall,
			.Depth = drawCall.SpriteInstanceData->Depth
		};

		screenSpaceItems.push_back(item);
	}

	for (const auto& drawCall : TextDrawer::Get().GetTextDrawCalls())
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
		.RenderSettings = &myRenderSettings,
		.CascadeShadows = &myShadowRenderPass->GetCascades()
	};
		
	myShadowRenderPass->Execute(inoutCommandList, myShadowItems, context);

	//// TODO: Run SSAO Before Lights
	
	inoutCommandList.BeginEvent("Main Pass");
	{
		FrameTargets targets =
		{
			.RenderTarget = myHDRBuffer.get(),
			.DepthBuffer = myDepthBuffer.get()
		};

		myDeferredRenderPass->Execute(inoutCommandList, myDeferredItems, targets, context);
		myForwardRenderPass->Execute(inoutCommandList, myForwardItems, targets, context);
	}
	inoutCommandList.EndEvent();

	{
		FrameTargets targets =
		{
			.RenderTarget = myBackBuffer.get(),
			.DepthBuffer = myDepthBuffer.get()
		};

		myScreenSpaceEffectPass->Execute(inoutCommandList, targets, context, myHDRBuffer.get());
		myDebugRenderPass->Execute(inoutCommandList, targets, context);
		myScreenSpaceRenderPass->Execute(inoutCommandList, screenSpaceItems, targets, context);
	}

	frameStats.PipelineSwitches = inoutCommandList.GetPipelineSwitcheCounter();
	return frameStats;
}

void GraphicsEngine::Present() const
{
	if (myRenderOverlay != nullptr)
	{
		myRHI->BindRenderTarget(myBackBuffer.get());
		myRenderOverlay->Render();
	}

	myRHI->Present();
}

void GraphicsEngine::ToggleBloom()
{
	myRenderSettings.BloomEnabled = !myRenderSettings.BloomEnabled;
	myScreenSpaceEffectPass->SetBloomStatus(myRenderSettings.BloomEnabled);
}

void GraphicsEngine::CycleDebugRenderMode(int aDirection)
{
	int nextDebugRenderMode = static_cast<int>(myRenderSettings.DebugRenderMode) + aDirection;

	if (nextDebugRenderMode >= static_cast<int>(DebugRenderMode::MAX))
	{
		nextDebugRenderMode = 0;
	}
	else if (nextDebugRenderMode < 0)
	{
		nextDebugRenderMode = static_cast<int>(DebugRenderMode::MAX) - 1;
	}

	myRenderSettings.DebugRenderMode = static_cast<DebugRenderMode>(nextDebugRenderMode);
}

void GraphicsEngine::CycleToNextTonemap(int aDirection)
{
	int nextToneMappingOperator = static_cast<int>(myRenderSettings.ToneMapper) + aDirection;

	if (nextToneMappingOperator >= static_cast<int>(ToneMapper::MAX))
	{
		nextToneMappingOperator = 0;
	}
	else if (nextToneMappingOperator < 0)
	{
		nextToneMappingOperator = static_cast<int>(ToneMapper::MAX) - 1;
	}

	myRenderSettings.ToneMapper = static_cast<ToneMapper>(nextToneMappingOperator);
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

void GraphicsEngine::SetRenderOverlay(RenderOverlay* aOverlay)
{
	myRenderOverlay = aOverlay;

	if (myRenderOverlay == nullptr)
	{
		return;
	}

	myRenderOverlay->OnAttach(myRHI->GetDevice(), myRHI->GetContext());
}

void GraphicsEngine::ClearRenderOverlay()
{
	if (myRenderOverlay == nullptr)
	{
		return;
	}

	myRenderOverlay->OnDetach();
	myRenderOverlay = nullptr;
}

void GraphicsEngine::SetSettings(const RenderSettings& aSettings)
{
	myRenderSettings = aSettings;
}

const RenderSettings& GraphicsEngine::GetSettings() const
{
	return myRenderSettings;
}

bool GraphicsEngine::InitializeRenderResources()
{
	// Trilinear Wrap
	{
		SamplerDescription samplerDesc;
		samplerDesc.Name = "TrilinearWrap";
		samplerDesc.AddressMode = SamplerAddressMode::Wrap;
		samplerDesc.FilterMode = SamplerFilterMode::Trilinear;

		std::unique_ptr<Sampler> sampler = std::make_unique<Sampler>();

		if (!myRHI->CreateSampler(samplerDesc, *sampler))
		{
			spdlog::critical("Failed to create Trilinear Wrap sampler!");
			return false;
		}

		mySamplers.emplace_back(std::move(sampler));
		mySamplerPointers.push_back(mySamplers.back().get());
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
			spdlog::critical("Failed to create Shadow sampler!");
			return false;
		}

		mySamplers.emplace_back(std::move(sampler));
		mySamplerPointers.push_back(mySamplers.back().get());
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
			spdlog::critical("Failed to create LUT sampler!");
			return false;
		}

		mySamplers.emplace_back(std::move(sampler));
		mySamplerPointers.push_back(mySamplers.back().get());
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
			spdlog::critical("Failed to create Trilinear Clamp sampler!");
			return false;
		}

		mySamplers.emplace_back(std::move(sampler));
		mySamplerPointers.push_back(mySamplers.back().get());
	}

	// LUT Texture
	{
		PipelineStateDescription lutPSODesc = {};
		lutPSODesc.Name = "LUTPSO";
		lutPSODesc.Topology = Topology::TriangleStrip;
		lutPSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		lutPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		lutPSODesc.PixelShader.ByteCode = INTERNAL_BRDF_LUT_PS_ByteCode;
		lutPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_BRDF_LUT_PS_ByteCode);

		PipelineStateObject lutPSO;

		if (!myRHI->CreatePipelineStateObject(lutPSODesc, lutPSO))
		{
			spdlog::critical("Failed to create LUT pipeline state object!");
			return false;
		}

		myLUTTexture = std::make_unique<Texture>();

		if (!myRHI->CreateTexture("BRDF_LUT", 512, 512, 1, ResourceFormat::R16G16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myLUTTexture))
		{
			spdlog::critical("Failed to create BRDF LUT texture!");
			return false;
		}

		GraphicsCommandList initCommandList;

		if (!myRHI->CreateCommandList("LUT Init", initCommandList))
		{
			spdlog::critical("Failed to create LUT init command list!");
			return false;
		}

		initCommandList.SetPipelineStateObject(lutPSO);
		initCommandList.SetRenderTarget(myLUTTexture.get(), nullptr);
		initCommandList.Draw(4);
		initCommandList.FinishCommandList();

		myRHI->ExecuteCommandList(initCommandList);
	}

	// HDR Buffer Texture
	{
		myHDRBuffer = std::make_unique<Texture>();

		CommonUtilities::Vector2ui clientSize = GetClientSize();

		if (!myRHI->CreateTexture("HDRBuffer", clientSize.x, clientSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myHDRBuffer))
		{
			spdlog::critical("Failed to create HDR buffer texture!");
			return false;
		}
	}

	return true;
}
