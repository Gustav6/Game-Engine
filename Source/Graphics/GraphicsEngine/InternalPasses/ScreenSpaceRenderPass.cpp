#include "GraphicsEngine.pch.h"
#include "ScreenSpaceRenderPass.h"

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "CommonUtilities/Math/Vector2.hpp"

#include "GraphicsEngine/Core/FrameTargets.h"
#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/Core/RenderContext.h"

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
	FrameTargets& aFrameTargets,
	RenderContext& aContext)
{
	inoutCommandList.BeginEvent("Screen Space Render");

	inoutCommandList.SetRenderTarget(aFrameTargets.BackBuffer, nullptr);

	const CommonUtilities::Vector2ui windowSize = aContext.RHI->GetClientSize();

	CommonUtilities::Matrix4x4f scale;
	scale.SetScaling({ 2.0f / static_cast<float>(windowSize.x), -2.0f / static_cast<float>(windowSize.y), 1.0f });

	CommonUtilities::Matrix4x4f translation;
	translation.SetTranslation({ -1.0f, 1.0f, 0.0f });

	const CommonUtilities::Matrix4x4f pixelToNDC = scale * translation;

	{
		inoutCommandList.BeginEvent("Text Render");

		FrameBuffer frameBuffer;
		frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
		frameBuffer.Projection = aContext.Camera->RenderCamera->GetProjection();
		frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();

		aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

		for (auto& drawCall : TextDrawer::Get().myDrawCalls)
		{
			inoutCommandList.BeginEvent(drawCall.Text->myFont->myName);

			if (drawCall.Text->myIsDirty)
			{
				drawCall.Text->RebuildGeometry();
				drawCall.Text->RebuildTextRenderMatrix();
				std::string vertexBufferName = std::format("{}_VX", drawCall.Text->myFont->myName);
				aContext.RHI->CreateVertexBuffer(vertexBufferName, drawCall.Text->myVertices, drawCall.Text->myVertexBuffer);
				std::string indexBufferName = std::format("{}_PX", drawCall.Text->myFont->myName);
				aContext.RHI->CreateIndexBuffer(indexBufferName, drawCall.Text->myIndices, drawCall.Text->myIndexBuffer);
				drawCall.Text->myIsDirty = false;
			}

			ObjectBuffer objectBuffer;
			objectBuffer.World = drawCall.Text->GetRenderMatrix() * pixelToNDC;
			objectBuffer.WorldInvT = objectBuffer.World.GetFastInverse().GetTranspose();

			aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader | PipelineStage_PixelShader);

			inoutCommandList.SetShaderResource(&drawCall.Text->myFont->myAtlas, 0, PipelineStage_PixelShader);

			const MaterialInterface* material = drawCall.Text->myFontMaterial.get();

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

			inoutCommandList.SetVertexBuffer(&drawCall.Text->myVertexBuffer);
			inoutCommandList.SetIndexBuffer(&drawCall.Text->myIndexBuffer);
			inoutCommandList.DrawIndexed(static_cast<unsigned int>(drawCall.Text->myIndices.size()), 0);

			inoutCommandList.EndEvent();
		}

		inoutCommandList.EndEvent();
	}

	{
		inoutCommandList.BeginEvent("Sprite Render");

		FrameBuffer frameBuffer;
		frameBuffer.Projection = mySpriteRenderResources.Projection;
		aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

		mySpriteRenderResources.Vertices.clear();

		const CommonUtilities::Vector2f parentSize =
		{
			1920.0f,
			1080.0f
		};

		for (auto& drawCall : SpriteDrawer::Get().myDrawCalls)
		{
			inoutCommandList.BeginEvent(drawCall.SpriteSharedData->Name);

			if (drawCall.SpriteSharedData == nullptr || drawCall.SpriteInstanceData == nullptr)
			{
				GELOG(Error, "Sprite is not valid!");
				continue;
			}

			const CommonUtilities::Vector2f position =
			{
				parentSize.x * drawCall.SpriteInstanceData->Anchor.x + drawCall.SpriteInstanceData->Position.x - drawCall.SpriteInstanceData->Size.x * drawCall.SpriteInstanceData->Pivot.x,
				parentSize.y * drawCall.SpriteInstanceData->Anchor.y + drawCall.SpriteInstanceData->Position.y - drawCall.SpriteInstanceData->Size.y * drawCall.SpriteInstanceData->Pivot.y
			};

			const CommonUtilities::Vector2f size = drawCall.SpriteInstanceData->Size;

			const float left = position.x;
			const float top = position.y;
			const float right = left + size.x;
			const float bottom = top + size.y;

			Vertex v0 =
			{
				.Position = { left,  top,    drawCall.SpriteInstanceData->Z, 1.0f },
				.UV0 = { 0.0f, 0.0f }
			};

			Vertex v1 =
			{
				.Position = { right, top,    drawCall.SpriteInstanceData->Z, 1.0f },
				.UV0 = { 1.0f, 0.0f }
			};

			Vertex v2 =
			{
				.Position = { left,  bottom, drawCall.SpriteInstanceData->Z, 1.0f },
				.UV0 = { 0.0f, 1.0f }
			};

			Vertex v3 =
			{
				.Position = { right, bottom, drawCall.SpriteInstanceData->Z, 1.0f },
				.UV0 = { 1.0f, 1.0f }
			};

			v0.Color = drawCall.SpriteInstanceData->Color;
			v1.Color = drawCall.SpriteInstanceData->Color;
			v2.Color = drawCall.SpriteInstanceData->Color;
			v3.Color = drawCall.SpriteInstanceData->Color;

			mySpriteRenderResources.Vertices.push_back(v0);
			mySpriteRenderResources.Vertices.push_back(v1);
			mySpriteRenderResources.Vertices.push_back(v2);
			mySpriteRenderResources.Vertices.push_back(v1);
			mySpriteRenderResources.Vertices.push_back(v3);
			mySpriteRenderResources.Vertices.push_back(v2);

			inoutCommandList.SetShaderResource(&drawCall.SpriteSharedData->Texture, 0, PipelineStage_PixelShader);

			const MaterialInterface* material = nullptr;

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

		inoutCommandList.EndEvent();
	}

	inoutCommandList.EndEvent();
}
