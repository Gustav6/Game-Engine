#include "GraphicsEngine.pch.h"
#include "ShadowRenderPass.h"

#include "CommonUtilities/Collision/Frustum.hpp"

#include "Shared/DrawCallSubmitter.h"

#include "Resources/GraphicsResourceFactory.h"

#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/Core/RenderContext.h"
#include "GraphicsEngine/Core/RenderEnums.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

#include "GraphicsEngine/ConstantBuffers/FrameBuffer.h"
#include "GraphicsEngine/ConstantBuffers/CascadeBuffer.h"

#include "GraphicsEngine/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "GraphicsEngine/Objects/GPU/Texture.h"
#include "GraphicsEngine/Objects/GPU/Vertex.h"

#include "GraphicsEngine/Objects/Scene/Light.h"
#include "GraphicsEngine/Objects/Scene/Camera.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "GraphicsEngine/Drawers/LightDrawer.h"

#include "RHI/PipelineStateObject.h"

#include "ShadowCube_GS.h"
#include "Surface_VS.h"
#include "Unlit_PS.h"

#include <tracy/core/Tracy.hpp>

bool ShadowRenderPass::Initialize(RenderHardwareInterface& aRHI)
{
	// Directional & Spot PSO
	{
		PipelineStateDescription shadowPSODesc = {};
		shadowPSODesc.Name = "ShadowPSO";
		shadowPSODesc.InputLayoutElements = Vertex::Description;
		shadowPSODesc.RasterizerState = RasterizerState::FillSolidCullFront;
		shadowPSODesc.VertexShader.ByteCode = INTERNAL_Surface_VS_ByteCode;
		shadowPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Surface_VS_ByteCode);

		myShadowPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(shadowPSODesc, *myShadowPSO))
		{
			GELOG(Error, "Failed to create Shadow pipeline state object!");
			return false;
		}
	}

	// Point PSO
	{
		PipelineStateDescription shadowCubePSODesc = {};
		shadowCubePSODesc.Name = "ShadowCubePSO";
		shadowCubePSODesc.GeometryShader.ByteCode = INTERNAL_ShadowCube_GS_ByteCode;
		shadowCubePSODesc.GeometryShader.ByteCodeSize = sizeof(INTERNAL_ShadowCube_GS_ByteCode);

		myShadowCubePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(shadowCubePSODesc, *myShadowCubePSO))
		{
			GELOG(Error, "Failed to create Shadow Cube pipeline state object!");
			return false;
		}
	}

	myCameraNearPlane = 0.0f;
	myCameraFarPlane = 0.0f;
	myShadowCascadePlaneChangeThreshold = 1.0f;

	return true;
}

void ShadowRenderPass::Execute(
	GraphicsCommandList& inoutCommandList,
	const std::vector<RenderItem>& aItems,
	RenderContext& aContext)
{
	ZoneScopedN("Shadow render pass")

	inoutCommandList.BeginEvent("Shadow Pass");

	inoutCommandList.SetPipelineStateObject(*myShadowPSO);

	for (const auto& light : LightDrawer::Get().mySceneLights)
	{
		switch (light.LightData->Type)
		{
			case LightType::Directional:
			{
				// TODO: Blend between cascades
				// https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps

				float cameraNearPlane = aContext.Camera->RenderCamera.GetNearPlane();
				float cameraFarPlane = aContext.Camera->RenderCamera.GetFarPlane();
				float cameraFov = aContext.Camera->RenderCamera.GetFovInDegrees();
				float cameraAspectRatio = aContext.Camera->RenderCamera.GetAspectRatio();

				bool needToRebuildCascades = false;

				if (std::abs(myCameraNearPlane - cameraNearPlane) > myShadowCascadePlaneChangeThreshold)
				{
					needToRebuildCascades = true;
				}
				else if (std::abs(myCameraFarPlane - cameraFarPlane) > myShadowCascadePlaneChangeThreshold)
				{
					needToRebuildCascades = true;
				}

				if (needToRebuildCascades)
				{
					myCameraNearPlane = cameraNearPlane;
					myCameraFarPlane = cameraFarPlane;
					BuildShadowCascades(aContext, { 4096, 2048, 2048, 2048 }, myCameraNearPlane, myCameraFarPlane);
				}

				CommonUtilities::Matrix4x4f lightViewProjectionTextures[NUM_CASCADES];

				CommonUtilities::Matrix4x4f cameraView = aContext.Camera->Transform.GetFastInverse();

				for (size_t c = 0; c < NUM_CASCADES; c++)
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

						aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader);

						DrawCallSubmitter::RenderDrawCalls(inoutCommandList, aItems, aContext, DrawCallFlag_None);

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
				cascadeBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();
				memcpy_s(cascadeBuffer.LightViewProjectionTextures, sizeof(CommonUtilities::Matrix4x4f) * NUM_CASCADES, lightViewProjectionTextures, sizeof(lightViewProjectionTextures));

				aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::CascadeBuffer, cascadeBuffer, 4, PipelineStage_PixelShader);

				break;
			}
			case LightType::Spot:
			{
				FrameBuffer frameBuffer;
				frameBuffer.Projection = light.Projection;
				frameBuffer.View = light.Transform.GetFastInverse();
				frameBuffer.CameraPosition = light.Transform.GetTranslation();

				inoutCommandList.BeginEvent("Spot Light Shadow Pass");

				inoutCommandList.ClearDepthStencil(*light.ShadowMap);
				inoutCommandList.SetRenderTarget(nullptr, light.ShadowMap);

				aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader);

				DrawCallSubmitter::RenderDrawCalls(inoutCommandList, aItems, aContext, DrawCallFlag_None);

				inoutCommandList.EndEvent();

				break;
			}
			case LightType::Point:
			{
				FrameBuffer frameBuffer;
				frameBuffer.Projection = light.Projection;
				frameBuffer.View = light.Transform.GetFastInverse();
				frameBuffer.CameraPosition = light.Transform.GetTranslation();

				inoutCommandList.BeginEvent("Point Light Shadow Pass");

				inoutCommandList.ClearDepthStencil(*light.ShadowMap);
				inoutCommandList.SetRenderTarget(nullptr, light.ShadowMap);

				inoutCommandList.SetOverridePipelineState(*myShadowCubePSO, PipelineStage_GeometryShader);

				aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_GeometryShader);

				DrawCallSubmitter::RenderDrawCalls(inoutCommandList, aItems, aContext, DrawCallFlag_None);

				inoutCommandList.ClearOverridePipelineState();

				inoutCommandList.EndEvent();

				break;
			}
		}
	}

	inoutCommandList.EndEvent();
}

const std::array<ShadowCascade, ShadowRenderPass::NUM_CASCADES>& ShadowRenderPass::GetCascades()
{
	return myCascades;
}

void ShadowRenderPass::BuildShadowCascades(
	RenderContext& aContext, 
	const std::array<int, 
	ShadowRenderPass::NUM_CASCADES>& aCascadeSizes, 
	float aCameraNear, 
	float aCameraFar)
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

		GraphicsResourceFactory::CreateShadowMap(*aContext.RHI, name, aCascadeSizes[c], aCascadeSizes[c], *myCascades[c].ShadowMap, false);
	}
}
