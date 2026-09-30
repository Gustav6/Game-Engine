#include "GraphicsEngine.pch.h"
#include "DeferredRenderPass.h"

#include "ShadowRenderPass.h"
#include "Shared/DrawCallSubmitter.h"

#include "GraphicsEngine/Core/RenderItem.h"
#include "GraphicsEngine/Core/FrameTargets.h"
#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/Core/RenderContext.h"
#include "GraphicsEngine/Core/RenderEnums.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

#include "GraphicsEngine/ConstantBuffers/LightBuffer.h"
#include "GraphicsEngine/ConstantBuffers/FrameBuffer.h"

#include "GraphicsEngine/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "GraphicsEngine/Objects/GPU/GBuffer.h"
#include "GraphicsEngine/Objects/Scene/Light.h"
#include "GraphicsEngine/Objects/Scene/Camera.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"
#include <GraphicsEngine/RHI/PipelineStateObject.h>

#include "GraphicsEngine/Drawers/LightDrawer.h"

#include "Deferred_DirLight_PS.h"
#include "Deferred_SpotLight_PS.h"
#include "Deferred_PointLight_PS.h"
#include "Deferred_Ambient_PS.h"
#include "FullTexture_VS.h"

#include <tracy/core/Tracy.hpp>

bool DeferredRenderPass::Initialize(RenderHardwareInterface& aRHI)
{
	// GBuffer
	{
		myGBuffer = std::make_unique<GBuffer>();

		CommonUtilities::Vector2ui renderSize = aRHI.GetClientSize();

		aRHI.CreateGBuffer("GBuffer", renderSize.x, renderSize.y, *myGBuffer);
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

		if (!aRHI.CreatePipelineStateObject(deferredDirLightDesc, *myDeferredAmbientPSO))
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

		if (!aRHI.CreatePipelineStateObject(deferredDirLightDesc, *myDeferredDirPSO))
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

		if (!aRHI.CreatePipelineStateObject(deferredSpotLightDesc, *myDeferredSpotPSO))
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

		if (!aRHI.CreatePipelineStateObject(deferredPointLightDesc, *myDeferredPointPSO))
		{
			GELOG(Error, "Failed to create deferred point light pipeline state object!");
			return false;
		}
	}

	return true;
}

void DeferredRenderPass::Execute(
	GraphicsCommandList& inoutCommandList,
	const std::vector<RenderItem>& aItems,
	FrameTargets& aFrameTargets,
	RenderContext& aContext)
{
	ZoneScopedN("Deferred Render Pass");

	inoutCommandList.BeginEvent("Deffered Render");

	inoutCommandList.BeginEvent("Clear GBuffer");
	inoutCommandList.ClearRenderTargets(myGBuffer->GetTextures(), GBuffer::NUM_GBUFFER_TARGETS);
	inoutCommandList.EndEvent();

	FrameBuffer frameBuffer;
	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
	frameBuffer.Projection = aContext.Camera->RenderCamera->GetProjection();
	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();
	frameBuffer.TypeOfRenderPass = static_cast<unsigned int>(aContext.Mode);

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	{
		inoutCommandList.BeginEvent("Object GBuffer Render");

		inoutCommandList.SetRenderTargets(myGBuffer->GetTextures(), GBuffer::NUM_GBUFFER_TARGETS, aFrameTargets.DepthBuffer);
		DrawCallSubmitter::RenderDrawCalls(inoutCommandList, aItems, aContext, DrawCallFlag_BindTextures | DrawCallFlag_BindMaterial);

		inoutCommandList.EndEvent();
	}

	{
		inoutCommandList.BeginEvent("Light Fullscreen Render");

		{
			inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, nullptr);

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

			if (aContext.Mode == DebugRenderMode::sRGB)
			{
				for (unsigned int l = 0; l < LightDrawer::Get().mySceneLights.size(); l++)
				{
					LightBuffer lightBuffer;
					lightBuffer.Lights[0] = *LightDrawer::Get().mySceneLights[l].LightData;
					aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::LightBuffer, lightBuffer, 3, PipelineStage_PixelShader);

					switch (LightDrawer::Get().mySceneLights[l].LightData->Type)
					{
						case LightType::Directional:
						{
							inoutCommandList.BeginEvent("Directional Light");
							std::vector<const Texture*> dirLightShadowMaps;
							dirLightShadowMaps.reserve(ShadowRenderPass::NUM_CASCADES);

							for (size_t c = 0; c < aContext.CascadeShadows->size(); c++)
							{
								dirLightShadowMaps.emplace_back(aContext.CascadeShadows->at(c).ShadowMap.get());
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

							const Texture* shadowMap = LightDrawer::Get().mySceneLights[l].ShadowMap;

							inoutCommandList.SetShaderResource(shadowMap, 104, PipelineStage_PixelShader);

							inoutCommandList.SetPipelineStateObject(*myDeferredSpotPSO);
							inoutCommandList.Draw(4);

							inoutCommandList.EndEvent();
							break;
						}
						case LightType::Point:
						{
							inoutCommandList.BeginEvent("Point Light");

							const Texture* shadowMap = LightDrawer::Get().mySceneLights[l].ShadowMap;

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
}