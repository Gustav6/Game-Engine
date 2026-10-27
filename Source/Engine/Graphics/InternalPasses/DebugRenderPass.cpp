#include "GraphicsEngine.pch.h"
#include "DebugRenderPass.h"

#include "GraphicsEngine.hpp"

#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/SceneCamera.h"
#include "Graphics/Core/FrameTargets.h"
#include "Graphics/Core/RenderEnums.h"

#include "Graphics/RHI/RenderHardwareInterface.h"

#include "Graphics/ConstantBuffers/FrameBuffer.h"
#include "Graphics/ConstantBuffers/ObjectBuffer.h"

#include "Graphics/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "Graphics/RHI/GraphicsCommandList.h"
#include "Graphics//RHI/PipelineStateObject.h"

#include "Graphics/Objects/Scene/Camera.h"
#include "Graphics/Objects/GPU/Vertex.h"
#include "Graphics/Objects/Mesh/Mesh.h"

#include "Graphics/Drawers/DebugDrawer.h"

#include "Surface_VS.h"
#include "Unlit_PS.h"
#include "Line_PS.h"
#include "Line_VS.h"

constexpr size_t MaxLineVertexPerDraw = 1000;

bool DebugRenderPass::Initialize(RenderHardwareInterface& aRHI)
{
	aRHI.CreateDynamicVertexBuffer("LineRenderer", sizeof(LineVertex), MaxLineVertexPerDraw, myLineVertexBuffer);

	// Wireframe Mesh PSO
	{
		PipelineStateDescription wireframePSODesc;
		wireframePSODesc.Name = "Wireframe Mesh";
		wireframePSODesc.VertexShader.ByteCode = INTERNAL_Surface_VS_ByteCode;
		wireframePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Surface_VS_ByteCode);
		wireframePSODesc.PixelShader.ByteCode = INTERNAL_Unlit_PS_ByteCode;
		wireframePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Unlit_PS_ByteCode);
		wireframePSODesc.InputLayoutElements = Vertex::Description;
		wireframePSODesc.BlendMode = BlendMode::Opaque;
		wireframePSODesc.RasterizerState = RasterizerState::FillWireframeCullNone;

		myDebugPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(wireframePSODesc, *myDebugPSO))
		{
			SPDLOG_CRITICAL("Failed to create Wireframe pipeline state object!");
			return false;
		}
	}

	// Line PSO
	{
		PipelineStateDescription linePSODesc;
		linePSODesc.Name = "Line";
		linePSODesc.VertexShader.ByteCode = INTERNAL_Line_VS_ByteCode;
		linePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Line_VS_ByteCode);
		linePSODesc.PixelShader.ByteCode = INTERNAL_Line_PS_ByteCode;
		linePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Line_PS_ByteCode);
		linePSODesc.InputLayoutElements = LineVertex::Description;
		linePSODesc.Topology = Topology::LineList;

		myLinePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(linePSODesc, *myLinePSO))
		{
			SPDLOG_CRITICAL("Failed to create Line pipeline state object!");
			return false;
		}
	}

	return true;
}

void DebugRenderPass::Execute(
	GraphicsCommandList& inoutCommandList, 
	FrameTargets& aFrameTargets,
	RenderContext& aContext)
{
	if (aContext.RenderSettings->DebugRenderMode != DebugRenderMode::None)
	{
		return;
	}

	inoutCommandList.BeginEvent("Debug Render");

	inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, aFrameTargets.DepthBuffer);

	FrameBuffer frameBuffer;
	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
	frameBuffer.Projection = aContext.Camera->RenderCamera.GetProjection();
	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();
	frameBuffer.TypeOfRenderPass = static_cast<unsigned int>(aContext.RenderSettings->DebugRenderMode);

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	RenderWireMeshes(inoutCommandList, aContext);
	RenderLines(inoutCommandList, aContext);

	inoutCommandList.EndEvent();
}

void DebugRenderPass::RenderWireMeshes(GraphicsCommandList& inoutCommandList, RenderContext& aContext)
{
	const std::vector<WireMeshDrawCall>& wireCalls = DebugDrawer::Get().GetWireframeDrawCalls();

	if (wireCalls.empty())
	{
		return;
	}

	// TODO: Add support to color the wireframes!

	inoutCommandList.SetPipelineStateObject(*myDebugPSO);

	for (const auto& drawCall : wireCalls)
	{
		const Mesh& mesh = *drawCall.Mesh;

		inoutCommandList.BeginEvent(mesh.GetName());

		for (unsigned int e = 0; e < drawCall.Mesh->myElements.size(); e++)
		{
			const Mesh::Element& element = mesh.myElements[e];

			if (!mesh.myVertexBuffer.IsValid())
			{
				if (!aContext.RHI->CreateVertexBuffer(mesh.myName, mesh.myVertices, mesh.myVertexBuffer))
				{
					SPDLOG_WARN("Could not prepare mesh {}! Vertex Buffer could not be created!", mesh.myName);
					continue;
				}
			}

			if (!mesh.myIndexBuffer.IsValid())
			{
				if (!aContext.RHI->CreateIndexBuffer(mesh.myName, mesh.myIndices, mesh.myIndexBuffer))
				{
					SPDLOG_WARN("Could not prepare mesh {}! Index Buffer could not be created!", mesh.myName);
					continue;
				}
			}

			inoutCommandList.SetVertexBuffer(&mesh.myVertexBuffer);
			inoutCommandList.SetIndexBuffer(&mesh.myIndexBuffer);

			ObjectBuffer objectBuffer;
			objectBuffer.World = drawCall.Transform;
			objectBuffer.WorldInvT = (drawCall.Transform.GetInverse()).GetTranspose();

			aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader);

			inoutCommandList.DrawIndexed(element.NumIndices, element.IndexOffset);
		}

		inoutCommandList.EndEvent();
	}
}

void DebugRenderPass::RenderLines(GraphicsCommandList& inoutCommandList, RenderContext&)
{
	const std::vector<LineDrawCall>& lineCalls = DebugDrawer::Get().GetLineDrawCalls();

	if (lineCalls.empty())
	{
		return;
	}

	inoutCommandList.SetPipelineStateObject(*myLinePSO);

	myLineVertices.clear();
	myLineVertices.reserve(lineCalls.size() * 2);

	for (const LineDrawCall& call : lineCalls)
	{
		myLineVertices.push_back({ { call.Start.x, call.Start.y, call.Start.z, 1.f }, call.Color });
		myLineVertices.push_back({ { call.End.x,   call.End.y,   call.End.z,   1.f }, call.Color });
	}

	inoutCommandList.SetVertexBuffer(&myLineVertexBuffer);

	const size_t totalVertices = myLineVertices.size();

	for (size_t offset = 0; offset < totalVertices; offset += MaxLineVertexPerDraw)
	{
		const size_t count = std::min(MaxLineVertexPerDraw, totalVertices - offset);

		if (!inoutCommandList.UpdateDynamicVertexBuffer(
			myLineVertexBuffer,
			myLineVertices.data() + offset,
			count * sizeof(LineVertex)))
		{
			return;
		}

		inoutCommandList.DrawSpecificVertices(static_cast<unsigned int>(count), 0);
	}
}