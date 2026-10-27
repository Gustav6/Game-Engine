#include "GraphicsEngine.pch.h"
#include "DeferredRenderPass.h"

#include "ShadowRenderPass.h"
#include "Shared/DrawCallSubmitter.h"

#include "Graphics/Core/RenderItem.h"
#include "Graphics/Core/FrameTargets.h"
#include "Graphics/Core/SceneCamera.h"
#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/RenderEnums.h"

#include "Graphics/InternalOperations/PostProcessFilters.h"

#include "Graphics/RHI/RenderHardwareInterface.h"

#include "Graphics/ConstantBuffers/LightBuffer.h"
#include "Graphics/ConstantBuffers/FrameBuffer.h"

#include "Graphics/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "Graphics/Objects/GPU/GBuffer.h"
#include "Graphics/Objects/Scene/Light.h"
#include "Graphics/Objects/Scene/Camera.h"

#include "Graphics/RHI/GraphicsCommandList.h"
#include "Graphics/RHI/PipelineStateObject.h"

#include "Graphics/Drawers/LightDrawer.h"

#include "DirectionalLighting_PS.h"
#include "SpotLighting_PS.h"
#include "PointLighting_PS.h"
#include "AmbientLighting_PS.h"
#include "Fullscreen_VS.h"
#include "SSAO_PS.h"

#include <tracy/core/Tracy.hpp>

bool DeferredRenderPass::Initialize(RenderHardwareInterface& aRHI)
{
	mySSAOEnabled = true;

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
		deferredDirLightDesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		deferredDirLightDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		deferredDirLightDesc.PixelShader.ByteCode = INTERNAL_AmbientLighting_PS_ByteCode;
		deferredDirLightDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_AmbientLighting_PS_ByteCode);
		deferredDirLightDesc.BlendMode = BlendMode::Additive;
		deferredDirLightDesc.Topology = Topology::TriangleStrip;

		myDeferredAmbientPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(deferredDirLightDesc, *myDeferredAmbientPSO))
		{
			SPDLOG_CRITICAL("Failed to create Deferred Ambient light pipeline state object!");
			return false;
		}
	}

	// Deffered Dir Light PSO
	{
		PipelineStateDescription deferredDirLightDesc;
		deferredDirLightDesc.Name = "DeferredDirLight";
		deferredDirLightDesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		deferredDirLightDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		deferredDirLightDesc.PixelShader.ByteCode = INTERNAL_DirectionalLighting_PS_ByteCode;
		deferredDirLightDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_DirectionalLighting_PS_ByteCode);
		deferredDirLightDesc.BlendMode = BlendMode::Additive;
		deferredDirLightDesc.Topology = Topology::TriangleStrip;

		myDeferredDirPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(deferredDirLightDesc, *myDeferredDirPSO))
		{
			SPDLOG_CRITICAL("Failed to create Deferred Directional light pipeline state object!");
			return false;
		}
	}

	// Deffered Spot Light PSO
	{
		PipelineStateDescription deferredSpotLightDesc;
		deferredSpotLightDesc.Name = "DeferredSpotLight";
		deferredSpotLightDesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		deferredSpotLightDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		deferredSpotLightDesc.PixelShader.ByteCode = INTERNAL_SpotLighting_PS_ByteCode;
		deferredSpotLightDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_SpotLighting_PS_ByteCode);
		deferredSpotLightDesc.BlendMode = BlendMode::Additive;
		deferredSpotLightDesc.Topology = Topology::TriangleStrip;

		myDeferredSpotPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(deferredSpotLightDesc, *myDeferredSpotPSO))
		{
			SPDLOG_CRITICAL("Failed to create Deferred Spot light pipeline state object!");
			return false;
		}
	}

	// Deffered Point Light PSO
	{
		PipelineStateDescription deferredPointLightDesc;
		deferredPointLightDesc.Name = "DeferredPointLight";
		deferredPointLightDesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		deferredPointLightDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		deferredPointLightDesc.PixelShader.ByteCode = INTERNAL_PointLighting_PS_ByteCode;
		deferredPointLightDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_PointLighting_PS_ByteCode);
		deferredPointLightDesc.BlendMode = BlendMode::Additive;
		deferredPointLightDesc.Topology = Topology::TriangleStrip;

		myDeferredPointPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(deferredPointLightDesc, *myDeferredPointPSO))
		{
			SPDLOG_CRITICAL("Failed to create Deferred Point light pipeline state object!");
			return false;
		}
	}

	// SSAO PSO
	{
		PipelineStateDescription ssaoDesc;
		ssaoDesc.Name = "SSAO";
		ssaoDesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		ssaoDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		ssaoDesc.PixelShader.ByteCode = INTERNAL_SSAO_PS_ByteCode;
		ssaoDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_SSAO_PS_ByteCode);
		ssaoDesc.BlendMode = BlendMode::Additive;
		ssaoDesc.Topology = Topology::TriangleStrip;

		mySSAOPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(ssaoDesc, *mySSAOPSO))
		{
			SPDLOG_CRITICAL("Failed to create SSAO pipeline state object!");
			return false;
		}
	}

	myGBufferTextures.reserve(GBuffer::NUM_GBUFFER_TARGETS);

	for (size_t t = 0; t < GBuffer::NUM_GBUFFER_TARGETS; t++)
	{
		myGBufferTextures.emplace_back(&myGBuffer->GetTextures()[t]);
	}

	// SSAO Textures
	{
		myHalfSizeSSAOTexture = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize() / 2u;

		if (!aRHI.CreateTexture("SSAO", bufferSize.x, bufferSize.y, 1, ResourceFormat::R32_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myHalfSizeSSAOTexture))
		{
			SPDLOG_CRITICAL("Failed to create SSAO texture!");
			return false;
		}

		myHalfSizeBlurVSSAOTexture = std::make_unique<Texture>();

		if (!aRHI.CreateTexture("SSAO_Blurred_V", bufferSize.x, bufferSize.y, 1, ResourceFormat::R32_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myHalfSizeBlurVSSAOTexture))
		{
			SPDLOG_CRITICAL("Failed to create SSAO Blurred texture!");
			return false;
		}

		myHalfSizeBlurHSSAOTexture = std::make_unique<Texture>();

		if (!aRHI.CreateTexture("SSAO_Blurred_H", bufferSize.x, bufferSize.y, 1, ResourceFormat::R32_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myHalfSizeBlurHSSAOTexture))
		{
			SPDLOG_CRITICAL("Failed to create SSAO Blurred texture!");
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
	ZoneScopedN("Deferred Render Pass")

	inoutCommandList.BeginEvent("Deffered Render");

	const std::vector<SceneLight>& sceneLights = LightDrawer::Get().GetSceneLights();

	inoutCommandList.BeginEvent("Clear GBuffer");
	inoutCommandList.ClearRenderTargets(myGBuffer->GetTextures(), GBuffer::NUM_GBUFFER_TARGETS);
	inoutCommandList.EndEvent();

	inoutCommandList.BeginEvent("Clear SSAO");
	inoutCommandList.ClearRenderTarget(*myHalfSizeSSAOTexture);
	inoutCommandList.EndEvent();

	FrameBuffer frameBuffer;
	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
	frameBuffer.Projection = aContext.Camera->RenderCamera.GetProjection();
	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();
	frameBuffer.TypeOfRenderPass = static_cast<unsigned int>(aContext.RenderSettings->DebugRenderMode);
	frameBuffer.SSAOFlag = mySSAOEnabled;

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

			inoutCommandList.BeginEvent("Ambient Light");

			if (mySSAOEnabled)
			{
				inoutCommandList.SetRenderTarget(myHalfSizeSSAOTexture.get(), nullptr);
				inoutCommandList.SetShaderResource(myGBufferTextures[1], 1, PipelineStage_PixelShader);

				inoutCommandList.SetPipelineStateObject(*mySSAOPSO);
				inoutCommandList.Draw(4);

				if (aContext.RenderSettings->DebugRenderMode == DebugRenderMode::None)
				{
					aContext.PostProcessFilters->BlurHorizontal(inoutCommandList, *myHalfSizeSSAOTexture, *myHalfSizeBlurHSSAOTexture);
					aContext.PostProcessFilters->BlurVertical(inoutCommandList, *myHalfSizeBlurHSSAOTexture, *myHalfSizeBlurVSSAOTexture);

					inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, nullptr);
					inoutCommandList.SetShaderResource(myHalfSizeBlurVSSAOTexture.get(), 97, PipelineStage_PixelShader);
				}
				else
				{
					inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, nullptr);
					inoutCommandList.SetShaderResource(myHalfSizeSSAOTexture.get(), 97, PipelineStage_PixelShader);
				}
			}

			// GBuffer set resources
			{
				inoutCommandList.SetShaderResources(myGBufferTextures.data(), myGBufferTextures.size(), 0, PipelineStage_PixelShader);
			}

			inoutCommandList.SetPipelineStateObject(*myDeferredAmbientPSO);
			inoutCommandList.Draw(4);
			inoutCommandList.EndEvent();

			if (aContext.RenderSettings->DebugRenderMode == DebugRenderMode::None || aContext.RenderSettings->DebugRenderMode == DebugRenderMode::Shadows)
			{
				for (const auto& light : sceneLights)
				{
					LightBuffer lightBuffer;
					lightBuffer.Lights[0] = *light.LightData;
					aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::LightBuffer, lightBuffer, 3, PipelineStage_PixelShader);

					switch (light.LightData->Type)
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

							const Texture* shadowMap = light.ShadowMap;

							inoutCommandList.SetShaderResource(shadowMap, 104, PipelineStage_PixelShader);

							inoutCommandList.SetPipelineStateObject(*myDeferredSpotPSO);
							inoutCommandList.Draw(4);

							inoutCommandList.EndEvent();
							break;
						}
						case LightType::Point:
						{
							inoutCommandList.BeginEvent("Point Light");

							const Texture* shadowMap = light.ShadowMap;

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

void DeferredRenderPass::SetSSAOStatus(bool aEnabled)
{
	mySSAOEnabled = aEnabled;
}