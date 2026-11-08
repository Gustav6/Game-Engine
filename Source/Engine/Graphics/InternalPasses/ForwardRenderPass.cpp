#include "Graphics/GraphicsEngine.pch.h"
#include "ForwardRenderPass.h"

#include "ShadowRenderPass.h"
#include "Shared/DrawCallSubmitter.h"

#include "Graphics/GraphicsEngine.hpp"

#include "Graphics/Core/RenderItem.h"
#include "Graphics/Core/FrameTargets.h"
#include "Graphics/Core/SceneCamera.h"
#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/RenderEnums.h"

#include "Graphics/RHI/GraphicsCommandList.h"

#include "Graphics/ConstantBuffers/LightBuffer.h"
#include "Graphics/ConstantBuffers/FrameBuffer.h"

#include "Graphics/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "Graphics/Objects/Scene/Light.h"
#include "Graphics/Objects/Scene/Camera.h"

#include "Graphics/Drawers/LightDrawer.h"


void ForwardRenderPass::Execute(
	GraphicsCommandList& inoutCommandList,
	const std::vector<RenderItem>& aItems,
	FrameTargets& aFrameTargets,
	RenderContext& aContext)
{
	inoutCommandList.BeginEvent("Forward Render");

	const std::vector<SceneLight>& sceneLights = LightDrawer::Get().GetSceneLights();

	inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, aFrameTargets.DepthBuffer);

	FrameBuffer frameBuffer;
	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
	frameBuffer.Projection = aContext.Camera->RenderCamera.GetProjection();
	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();
	frameBuffer.TypeOfRenderPass = static_cast<unsigned int>(aContext.RenderSettings->DebugRenderMode);

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	LightBuffer lightBuffer;
	lightBuffer.NumLights = static_cast<unsigned int>(std::min(sceneLights.size(), static_cast<size_t>(MAX_LIGHTS)));

	std::vector<const Texture*> dirLightShadowMaps;
	dirLightShadowMaps.reserve(ShadowRenderPass::NUM_CASCADES);
	std::vector<const Texture*> spotLightShadowMaps;
	std::vector<const Texture*> pointLightShadowCubes;

	unsigned int spotShadowIndex = 0;
	unsigned int pointShadowIndex = 0;

	for (unsigned int i = 0; i < lightBuffer.NumLights; i++)
	{
		lightBuffer.Lights[i] = *sceneLights[i].LightData;

		switch (sceneLights[i].LightData->Type)
		{
			case LightType::Directional:
			{
				for (size_t c = 0; c < aContext.CascadeShadows->size(); c++)
				{
					dirLightShadowMaps.emplace_back(aContext.CascadeShadows->at(c).ShadowMap.get());
				}

				break;
			}
			case LightType::Spot:
			{
				if (sceneLights[i].ShadowMap == nullptr)
				{
					continue;
				}

				spotLightShadowMaps.emplace_back(LightDrawer::Get().GetSceneLights()[i].ShadowMap);
				lightBuffer.Lights[i].ShadowIndex = spotShadowIndex++;
				break;
			}
			case LightType::Point:
			{
				if (sceneLights[i].ShadowMap == nullptr)
				{
					continue;
				}

				pointLightShadowCubes.emplace_back(sceneLights[i].ShadowMap);
				lightBuffer.Lights[i].ShadowIndex = pointShadowIndex++;
				break;
			}
		}
	}

	inoutCommandList.SetShaderResources(dirLightShadowMaps.data(), dirLightShadowMaps.size(), 100, PipelineStage_PixelShader);
	inoutCommandList.SetShaderResources(spotLightShadowMaps.data(), spotLightShadowMaps.size(), 104, PipelineStage_PixelShader);
	inoutCommandList.SetShaderResources(pointLightShadowCubes.data(), pointLightShadowCubes.size(), 112, PipelineStage_PixelShader);

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::LightBuffer, lightBuffer, 3, PipelineStage_PixelShader);

	DrawCallSubmitter::RenderDrawCalls(inoutCommandList, aItems, aContext, DrawCallFlag_BindTextures | DrawCallFlag_BindMaterial);

	inoutCommandList.EndEvent();
}
