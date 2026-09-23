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

#include "GraphicsEngine/Objects/Text/Text.h"
#include "GraphicsEngine/Objects/Text/Font.h"

#include "GraphicsEngine/Objects/Scene/Camera.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"

#include "GraphicsEngine/Drawers/TextDrawer.h"

void ScreenSpaceRenderPass::Execute(
	GraphicsCommandList& inoutCommandList,
	FrameTargets& aFrameTargets,
	RenderContext& aContext)
{
	inoutCommandList.BeginEvent("Screen Space Render");

	inoutCommandList.SetRenderTarget(aFrameTargets.BackBuffer, nullptr);

	FrameBuffer frameBuffer;
	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
	frameBuffer.Projection = aContext.Camera->RenderCamera->GetProjection();
	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	const CommonUtilities::Vector2ui windowSize = aContext.RHI->GetClientSize();

	CommonUtilities::Matrix4x4f scale;
	scale.SetScaling({ 2.0f / static_cast<float>(windowSize.x), -2.0f / static_cast<float>(windowSize.y), 1.0f });

	CommonUtilities::Matrix4x4f translation;
	translation.SetTranslation({ -1.0f, 1.0f, 0.0f });

	const CommonUtilities::Matrix4x4f pixelToNDC = scale * translation;

	for (auto& drawCall : TextDrawer::Get().myDrawCalls)
	{
		inoutCommandList.BeginEvent(drawCall.Text->myName);

		if (drawCall.Text->myFont == nullptr)
		{
			GELOG(Error, "Text had no valid font!");
			continue;
		}

		if (drawCall.Text->myIsDirty)
		{
			drawCall.Text->RebuildGeometry();
			drawCall.Text->RebuildTextRenderMatrix();
			std::string vertexBufferName = std::format("{}_VX", drawCall.Text->myName);
			aContext.RHI->CreateVertexBuffer(vertexBufferName, drawCall.Text->myVertices, drawCall.Text->myVertexBuffer);
			std::string indexBufferName = std::format("{}_PX", drawCall.Text->myName);
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