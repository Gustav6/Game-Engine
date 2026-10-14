#include "GraphicsEngine.pch.h"
#include "ScreenSpaceRenderPass.h"

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector2.hpp"

#include "GraphicsEngine/Core/FrameTargets.h"
#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/Core/RenderContext.h"
#include "GraphicsEngine/Core/RenderEnums.h"
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

#include "FontMaterial_PS.h"
#include "FontMaterial_VS.h"

#include "Sprite_PS.h"
#include "Sprite_VS.h"

#include <tracy/core/Tracy.hpp>

bool ScreenSpaceRenderPass::Initialize(RenderHardwareInterface& aRHI)
{
	constexpr float designWidth = 1920.0f;
	constexpr float designHeight = 1080.0f;

	mySpriteProjection = CommonUtilities::Matrix4x4f::Identity;

	mySpriteProjection(1, 1) = 2.0f / designWidth;
	mySpriteProjection(2, 2) = -2.0f / designHeight;
	mySpriteProjection(4, 1) = -1.0f;
	mySpriteProjection(4, 2) = 1.0f;

	aRHI.CreateDynamicVertexBuffer("SpriteRenderer", 10000, mySpriteVertexBuffer);

	// Font PSO
	{
		PipelineStateDescription fontPSODesc = {};
		fontPSODesc.Name = "FontPSO";
		fontPSODesc.BlendMode = BlendMode::Alpha;
		fontPSODesc.Topology = Topology::TriangleList;
		fontPSODesc.VertexShader.ByteCode = INTERNAL_FontMaterial_VS_ByteCode;
		fontPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FontMaterial_VS_ByteCode);
		fontPSODesc.PixelShader.ByteCode = INTERNAL_FontMaterial_PS_ByteCode;
		fontPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_FontMaterial_PS_ByteCode);

		myFontPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(fontPSODesc, *myFontPSO))
		{
			spdlog::critical("Failed to create Font pipeline state object!");
			return false;
		}
	}

	// Sprite PSO
	{
		PipelineStateDescription spritePSODesc = {};
		spritePSODesc.Name = "SpritePSO";
		spritePSODesc.BlendMode = BlendMode::Alpha;
		spritePSODesc.Topology = Topology::TriangleList;
		spritePSODesc.VertexShader.ByteCode = INTERNAL_Sprite_VS_ByteCode;
		spritePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Sprite_VS_ByteCode);
		spritePSODesc.PixelShader.ByteCode = INTERNAL_Sprite_PS_ByteCode;
		spritePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Sprite_PS_ByteCode);

		mySpritePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(spritePSODesc, *mySpritePSO))
		{
			spdlog::critical("Failed to create Sprite pipeline state object!");
			return false;
		}
	}

	return true;
}

void ScreenSpaceRenderPass::Execute(
	GraphicsCommandList& inoutCommandList,
	const std::vector<ScreenSpaceItem>& aItems,
	FrameTargets& aFrameTargets,
	RenderContext& aContext)
{
	ZoneScopedN("Screen Space Render Pass")

	inoutCommandList.BeginEvent("Screen Space Render");

	inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, nullptr);

	for (size_t i = 0; i < aItems.size(); i++)
	{
		if (aItems[i].SpriteCall != nullptr)
		{
			inoutCommandList.SetOverridePipelineState(*mySpritePSO, PipelineStage_VertexShader | PipelineStage_PixelShader);
			RenderSprite(inoutCommandList, *aItems[i].SpriteCall, aContext);
		}
		if (aItems[i].TextCall != nullptr)
		{
			inoutCommandList.SetOverridePipelineState(*myFontPSO, PipelineStage_VertexShader | PipelineStage_PixelShader);
			RenderText(inoutCommandList, *aItems[i].TextCall, aContext);
		}
	}

	inoutCommandList.ClearOverridePipelineState();

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
	frameBuffer.Projection = aContext.Camera->RenderCamera.GetProjection();
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
		material = &aContext.BuiltinAssets->GetDefaultMaterial();
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
	frameBuffer.Projection = mySpriteProjection;
	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	mySpriteVertices.clear();

	const CommonUtilities::Vector2f parentSize =
	{
		1920.0f,
		1080.0f
	};

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

	mySpriteVertices.push_back(v0);
	mySpriteVertices.push_back(v1);
	mySpriteVertices.push_back(v2);
	mySpriteVertices.push_back(v1);
	mySpriteVertices.push_back(v3);
	mySpriteVertices.push_back(v2);

	inoutCommandList.SetShaderResource(&aDrawCall.SpriteSharedData->Texture, 0, PipelineStage_PixelShader);

	const MaterialInterface* material = aDrawCall.Material;

	if (material == nullptr)
	{
		material = &aContext.BuiltinAssets->GetDefaultMaterial();
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

	inoutCommandList.UpdateVertexBuffer(mySpriteVertexBuffer, mySpriteVertices);
	inoutCommandList.SetVertexBuffer(&mySpriteVertexBuffer);
	inoutCommandList.DrawSpecificVertices(static_cast<unsigned int>(mySpriteVertices.size()), 0);
}
