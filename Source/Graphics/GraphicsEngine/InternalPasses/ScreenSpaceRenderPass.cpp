#include "GraphicsEngine.pch.h"
#include "ScreenSpaceRenderPass.h"

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector2.hpp"

#include "GraphicsEngine/Core/FrameTargets.h"
#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/Core/RenderContext.h"
#include "GraphicsEngine/Core/RenderItem.h"

#include "GraphicsEngine/Resources/BuiltinAssets.h"

#include "RHI/RenderHardwareInterface.h"

#include "GraphicsEngine/Materials/MaterialInterface.h"

#include "GraphicsEngine/ConstantBuffers/FrameBuffer.h"
#include "GraphicsEngine/ConstantBuffers/ObjectBuffer.h"

#include "GraphicsEngine/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "GraphicsEngine/Objects/GPU/Vertex.h"

#include "GraphicsEngine/Objects/Text/Text.h"
#include "GraphicsEngine/Objects/Text/Font.h"

#include "GraphicsEngine/Objects/Sprite.h"

#include "GraphicsEngine/Objects/Scene/Camera.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "GraphicsEngine/Drawers/TextDrawer.h"
#include "GraphicsEngine/Drawers/SpriteDrawer.h"

ScreenSpaceRenderPass::ScreenSpaceRenderPass(RenderHardwareInterface& aRHI)
{
	constexpr float designWidth = 1920.0f;
	constexpr float designHeight = 1080.0f;

	mySpriteRenderResources.Projection = CommonUtilities::Matrix4x4f::Identity;

	mySpriteRenderResources.Projection(1, 1) = 2.0f / designWidth;
	mySpriteRenderResources.Projection(2, 2) = -2.0f / designHeight;
	mySpriteRenderResources.Projection(4, 1) = -1.0f;
	mySpriteRenderResources.Projection(4, 2) = 1.0f;

	aRHI.CreateDynamicVertexBuffer("SpriteRenderer", 10000, mySpriteRenderResources.VertexBuffer);
}

void ScreenSpaceRenderPass::Execute(
	GraphicsCommandList& inoutCommandList,
	const std::vector<ScreenSpaceItem>& aItems,
	FrameTargets& aFrameTargets,
	RenderContext& aContext)
{
	inoutCommandList.BeginEvent("Screen Space Render");

	inoutCommandList.SetRenderTarget(aFrameTargets.BackBuffer, nullptr);

	for (size_t i = 0; i < aItems.size(); i++)
	{
		if (aItems[i].SpriteCall != nullptr)
		{
			RenderSprite(inoutCommandList, *aItems[i].SpriteCall, aContext);
		}
		if (aItems[i].TextCall != nullptr)
		{
			RenderText(inoutCommandList, *aItems[i].TextCall, aContext);
		}
	}

	inoutCommandList.EndEvent();
}

void ScreenSpaceRenderPass::RenderText(
	GraphicsCommandList& inoutCommandList, 
	const TextDrawCall& aDrawCall, 
	RenderContext& aContext)
{
	const CommonUtilities::Vector2ui windowSize = aContext.RHI->GetClientSize();

	CommonUtilities::Matrix4x4f scale;
	scale.SetScaling({ 2.0f / static_cast<float>(windowSize.x), -2.0f / static_cast<float>(windowSize.y), 1.0f });

	CommonUtilities::Matrix4x4f translation;
	translation.SetTranslation({ -1.0f, 1.0f, 0.0f });

	const CommonUtilities::Matrix4x4f pixelToNDC = scale * translation;

	FrameBuffer frameBuffer;
	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
	frameBuffer.Projection = aContext.Camera->RenderCamera->GetProjection();
	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	inoutCommandList.BeginEvent(aDrawCall.Font->myName);

	if (aDrawCall.Text->myIsDirty)
	{
		aDrawCall.Text->RebuildGeometry(*aDrawCall.Font);
		std::string vertexBufferName = std::format("{}_VX", aDrawCall.Font->myName);
		aContext.RHI->CreateVertexBuffer(vertexBufferName, aDrawCall.Text->myVertices, aDrawCall.Text->myVertexBuffer);
		std::string indexBufferName = std::format("{}_PX", aDrawCall.Font->myName);
		aContext.RHI->CreateIndexBuffer(indexBufferName, aDrawCall.Text->myIndices, aDrawCall.Text->myIndexBuffer);
		aDrawCall.Text->myIsDirty = false;
	}

	ObjectBuffer objectBuffer;
	objectBuffer.World = aDrawCall.Transform * pixelToNDC;
	objectBuffer.WorldInvT = objectBuffer.World.GetFastInverse().GetTranspose();

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader | PipelineStage_PixelShader);

	inoutCommandList.SetShaderResource(&aDrawCall.Font->myAtlas, 0, PipelineStage_PixelShader);

	const MaterialInterface* material = aDrawCall.Material;

	if (material == nullptr)
	{
		material = &aContext.BuiltinAssets->GetDefaultFontMaterial();
	}

	if (material->HasParameters())
	{
		if (material->IsMaterialDataDirty())
		{
			material->RefreshMaterialData();
		}

		aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::MaterialBuffer, material->GetParameterDataBlock(), Material::MATERIAL_BUFFER_SIZE, 5, PipelineStage_VertexShader | PipelineStage_PixelShader);
	}

	inoutCommandList.SetPipelineStateObject(material->GetPSO());

	inoutCommandList.SetVertexBuffer(&aDrawCall.Text->myVertexBuffer);
	inoutCommandList.SetIndexBuffer(&aDrawCall.Text->myIndexBuffer);
	inoutCommandList.DrawIndexed(static_cast<unsigned int>(aDrawCall.Text->myIndices.size()), 0);

	inoutCommandList.EndEvent();
}

void ScreenSpaceRenderPass::RenderSprite(
	GraphicsCommandList& inoutCommandList, 
	const SpriteDrawCall& aDrawCall, 
	RenderContext& aContext)
{
	FrameBuffer frameBuffer;
	frameBuffer.Projection = mySpriteRenderResources.Projection;
	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	mySpriteRenderResources.Vertices.clear();

	const CommonUtilities::Vector2f parentSize =
	{
		1920.0f,
		1080.0f
	};

	inoutCommandList.BeginEvent(aDrawCall.SpriteSharedData->Name);

	const CommonUtilities::Vector2f position =
	{
		parentSize.x * aDrawCall.SpriteInstanceData->Anchor.x + aDrawCall.SpriteInstanceData->Position.x - aDrawCall.SpriteInstanceData->Size.x * aDrawCall.SpriteInstanceData->Pivot.x,
		parentSize.y * aDrawCall.SpriteInstanceData->Anchor.y + aDrawCall.SpriteInstanceData->Position.y - aDrawCall.SpriteInstanceData->Size.y * aDrawCall.SpriteInstanceData->Pivot.y
	};

	const CommonUtilities::Vector2f size = aDrawCall.SpriteInstanceData->Size;

	const float left = position.x;
	const float top = position.y;
	const float right = left + size.x;
	const float bottom = top + size.y;

	Vertex v0 =
	{
		.Position = { left, top, aDrawCall.SpriteInstanceData->Depth, 1.0f },
		.UV0 = { 0.0f, 0.0f }
	};

	Vertex v1 =
	{
		.Position = { right, top, aDrawCall.SpriteInstanceData->Depth, 1.0f },
		.UV0 = { 1.0f, 0.0f }
	};

	Vertex v2 =
	{
		.Position = { left,  bottom, aDrawCall.SpriteInstanceData->Depth, 1.0f },
		.UV0 = { 0.0f, 1.0f }
	};

	Vertex v3 =
	{
		.Position = { right, bottom, aDrawCall.SpriteInstanceData->Depth, 1.0f },
		.UV0 = { 1.0f, 1.0f }
	};

	v0.Color = aDrawCall.SpriteInstanceData->Color;
	v1.Color = aDrawCall.SpriteInstanceData->Color;
	v2.Color = aDrawCall.SpriteInstanceData->Color;
	v3.Color = aDrawCall.SpriteInstanceData->Color;

	mySpriteRenderResources.Vertices.push_back(v0);
	mySpriteRenderResources.Vertices.push_back(v1);
	mySpriteRenderResources.Vertices.push_back(v2);
	mySpriteRenderResources.Vertices.push_back(v1);
	mySpriteRenderResources.Vertices.push_back(v3);
	mySpriteRenderResources.Vertices.push_back(v2);

	inoutCommandList.SetShaderResource(&aDrawCall.SpriteSharedData->Texture, 0, PipelineStage_PixelShader);

	const MaterialInterface* material = aDrawCall.Material;

	if (material == nullptr)
	{
		material = &aContext.BuiltinAssets->GetDefaultSpriteMaterial();
	}

	if (material->HasParameters())
	{
		if (material->IsMaterialDataDirty())
		{
			material->RefreshMaterialData();
		}

		aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::MaterialBuffer, material->GetParameterDataBlock(), Material::MATERIAL_BUFFER_SIZE, 5, PipelineStage_VertexShader | PipelineStage_PixelShader);
	}

	inoutCommandList.SetPipelineStateObject(material->GetPSO());

	inoutCommandList.UpdateVertexBuffer(mySpriteRenderResources.VertexBuffer, mySpriteRenderResources.Vertices);
	inoutCommandList.SetVertexBuffer(&mySpriteRenderResources.VertexBuffer);
	inoutCommandList.DrawSpecificVertices(static_cast<unsigned int>(mySpriteRenderResources.Vertices.size()), 0);

	inoutCommandList.EndEvent();
}
