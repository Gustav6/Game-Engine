#include "GraphicsEngine.pch.h"
#include "AlphaRenderPass.h"

#include "ShadowRenderPass.h"
#include "Shared/RenderItem.h"
#include "Shared/FrameTargets.h"
#include "Shared/DrawCallSubmitter.h"
#include "Shared/SceneCamera.h"
#include "Shared/RenderContext.h"

#include "GraphicsEngine/GraphicsEngine.hpp"
#include "GraphicsEngine/DebugRenderMode.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "GraphicsEngine/ConstantBuffers/LightBuffer.h"
#include "GraphicsEngine/ConstantBuffers/FrameBuffer.h"

#include "GraphicsEngine/Objects/Light.h"
#include "GraphicsEngine/Objects/Camera.h"

#include "GraphicsEngine/Drawers/LightDrawer.h"

void AlphaRenderPass::Execute(
	GraphicsEngine& aGraphicsEngine,
	GraphicsCommandList& inoutCommandList,
	const std::vector<RenderItem>& aItems,
	FrameTargets& aFrameTargets,
	RenderContext& aContext)
{
	inoutCommandList.SetRenderTarget(aFrameTargets.BackBuffer, aFrameTargets.DepthBuffer);

	FrameBuffer frameBuffer;
	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
	frameBuffer.Projection = aContext.Camera->RenderCamera->GetProjection();
	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();
	frameBuffer.TypeOfRenderPass = static_cast<unsigned int>(aContext.Mode);

	aGraphicsEngine.UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	LightBuffer lightBuffer;
	lightBuffer.NumLights = static_cast<unsigned int>(std::min(LightDrawer::Get().mySceneLights.size(), static_cast<size_t>(MAX_LIGHTS)));

	std::vector<const Texture*> dirLightShadowMaps;
	dirLightShadowMaps.reserve(ShadowRenderPass::NUM_CASCADES);
	std::vector<const Texture*> spotLightShadowMaps;
	std::vector<const Texture*> pointLightShadowCubes;

	unsigned int spotShadowIndex = 0;
	unsigned int pointShadowIndex = 0;

	for (unsigned int i = 0; i < lightBuffer.NumLights; i++)
	{
		if (LightDrawer::Get().mySceneLights[i].LightData == nullptr)
		{
			continue;
		}

		lightBuffer.Lights[i] = *LightDrawer::Get().mySceneLights[i].LightData;

		switch (LightDrawer::Get().mySceneLights[i].LightData->Type)
		{
		case LightType::Directional:
		{
			for (size_t c = 0; c < aContext.CascadeShadows.size(); c++)
			{
				dirLightShadowMaps.emplace_back(aContext.CascadeShadows[c].ShadowMap.get());
			}

			break;
		}
		case LightType::Spot:
		{
			if (LightDrawer::Get().mySceneLights[i].ShadowMap == nullptr)
			{
				continue;
			}

			spotLightShadowMaps.emplace_back(LightDrawer::Get().mySceneLights[i].ShadowMap);
			lightBuffer.Lights[i].ShadowIndex = spotShadowIndex++;
			break;
		}
		case LightType::Point:
		{
			if (LightDrawer::Get().mySceneLights[i].ShadowMap == nullptr)
			{
				continue;
			}

			pointLightShadowCubes.emplace_back(LightDrawer::Get().mySceneLights[i].ShadowMap);
			lightBuffer.Lights[i].ShadowIndex = pointShadowIndex++;
			break;
		}
		}
	}

	inoutCommandList.SetShaderResources(dirLightShadowMaps.data(), dirLightShadowMaps.size(), 100, PipelineStage_PixelShader);
	inoutCommandList.SetShaderResources(spotLightShadowMaps.data(), spotLightShadowMaps.size(), 104, PipelineStage_PixelShader);
	inoutCommandList.SetShaderResources(pointLightShadowCubes.data(), pointLightShadowCubes.size(), 112, PipelineStage_PixelShader);

 	aGraphicsEngine.UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::LightBuffer, lightBuffer, 3, PipelineStage_PixelShader);

	DrawCallSubmitter::RenderDrawCalls(aGraphicsEngine, inoutCommandList, aItems, aContext, DrawCallFlag_BindTextures | DrawCallFlag_BindMaterial);
}
