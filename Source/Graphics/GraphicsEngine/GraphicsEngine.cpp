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
#include "InternalPasses/ScreenSpaceEffectPass.h"

#include "Core/RenderItem.h"
#include "Core/FrameTargets.h"
#include "Core/RenderContext.h"
#include "Core/RenderEnums.h"
#include "Core/SceneCamera.h"

#include "Drawers/MeshDrawer.h"
#include "Drawers/LightDrawer.h"
#include "Drawers/TextDrawer.h"
#include "Drawers/SpriteDrawer.h"

// Internal hlsl code
#include "FullTexture_VS.h"
#include "BRDF_LUT_PS.h"

#include <tracy/core/Tracy.hpp>

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
	}

	myConstantBufferManager = std::make_unique<ConstantBufferManager>();

	if (!InitializeRenderResources())
	{
		return false;
	}

	// Init passes
	{
		myForwardRenderPass = std::make_unique<ForwardRenderPass>();
		myDeferredRenderPass = std::make_unique<DeferredRenderPass>();
		myScreenSpaceRenderPass = std::make_unique<ScreenSpaceRenderPass>(*myRHI);
		myShadowRenderPass = std::make_unique<ShadowRenderPass>();
		myScreenSpaceEffectPass = std::make_unique<ScreenSpaceEffectPass>(*myRHI);

		myDeferredRenderPass->Initialize(*myRHI);
		myShadowRenderPass->Initialize(*myRHI);
	}

	myDebugRenderMode = DebugRenderMode::sRGB;
	myToneMappingOperator = ToneMappingOperator::Unreal;

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
	ZoneScopedN("Whole render pass");

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

	myOpaqueItems.clear();
	myAlphaItems.clear();
	myShadowItems.clear();

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
				myOpaqueItems.push_back(item);
			}
			else
			{
				switch (material->GetBlendMode())
				{
					case MaterialBlendMode::Opaque:
					{
						myOpaqueItems.push_back(item);
						myShadowItems.push_back(item);
						break;
					}
					case MaterialBlendMode::Alpha:
					{
						myAlphaItems.push_back(item);
						break;
					}
					default:
					{
						GELOG(Error, "Mesh {} has a invalid Material Blend Mode setting!", drawCall.Mesh->GetName());
						break;
					}
				}
			}
		}
	}

	myOpaqueItems.shrink_to_fit();
	myAlphaItems.shrink_to_fit();
	myShadowItems.shrink_to_fit();

	CommonUtilities::Vector3f cameraPosition = aCamera.Transform.GetTranslation();

	// Front to Back
	std::sort(myOpaqueItems.begin(), myOpaqueItems.end(),
		[cameraPosition](const RenderItem& A, const RenderItem& B) -> bool
		{
			float distanceToA = (A.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();
			float distanceToB = (B.DrawCall->Transform.GetTranslation() - cameraPosition).LengthSqr();

			return distanceToA < distanceToB;
		});

	// Back to Front
	std::sort(myAlphaItems.begin(), myAlphaItems.end(),
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
			.Depth = drawCall.SpriteInstanceData->Depth
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
		.Tonmap = myToneMappingOperator,
		.CascadeShadows = &myShadowRenderPass->GetCascades()
	};

	myShadowRenderPass->Execute(inoutCommandList, myShadowItems, context);

	inoutCommandList.BeginEvent("Main Pass");
	{
		FrameTargets targets =
		{
			.RenderTarget = myHDRBuffer.get(),
			.DepthBuffer = myDepthBuffer.get()
		};

		myDeferredRenderPass->Execute(inoutCommandList, myOpaqueItems, targets, context);
		myForwardRenderPass->Execute(inoutCommandList, myAlphaItems, targets, context);
	}
	inoutCommandList.EndEvent();

	{
		FrameTargets targets =
		{
			.RenderTarget = myBackBuffer.get(),
			.DepthBuffer = myDepthBuffer.get()
		};

		myScreenSpaceEffectPass->Execute(inoutCommandList, targets, context, myHDRBuffer.get());
		myScreenSpaceRenderPass->Execute(inoutCommandList, screenSpaceItems, targets, context);
	}
}

void GraphicsEngine::Present() const
{
	myRHI->Present();
}

void GraphicsEngine::ToggleBloom() const
{
	myScreenSpaceEffectPass->ToggleBloom();
}

void GraphicsEngine::CycleDebugRenderMode(int aDirection)
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

void GraphicsEngine::CycleToNextTonemap(int aDirection)
{
	int nextToneMappingOperator = static_cast<int>(myToneMappingOperator) + aDirection;

	if (nextToneMappingOperator >= static_cast<int>(ToneMappingOperator::MAX))
	{
		nextToneMappingOperator = 0;
	}
	else if (nextToneMappingOperator < 0)
	{
		nextToneMappingOperator = static_cast<int>(ToneMappingOperator::MAX) - 1;
	}

	myToneMappingOperator = static_cast<ToneMappingOperator>(nextToneMappingOperator);
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

	// HDR Buffer Texture
	{
		myHDRBuffer = std::make_unique<Texture>();

		CommonUtilities::Vector2ui clientSize = GetClientSize();

		if (!myRHI->CreateTexture("HDRBuffer", clientSize.x, clientSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myHDRBuffer))
		{
			GELOG(Error, "Failed to create HDR buffer texture!");
			return false;
		}
	}

	return true;
}