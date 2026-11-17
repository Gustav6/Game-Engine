#include "Graphics/GraphicsEngine.pch.h"
#include "DebugRenderPass.h"


#include "Graphics/GraphicsEngine.hpp"

#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/SceneCamera.h"
#include "Graphics/Core/FrameTargets.h"
#include "Graphics/Core/RenderEnums.h"

#include "Graphics/RHI/RenderHardwareInterface.h"

#include "Graphics/Buffers/FrameBuffer.h"
#include "Graphics/Buffers/ObjectBuffer.h"

#include "Graphics/Buffers/Manager/BufferManager.h"

#include "Graphics/RHI/GraphicsCommandList.h"
#include "Graphics//RHI/PipelineStateObject.h"

#include "Graphics/Objects/Scene/Camera.h"
#include "Graphics/Objects/GPU/Vertex.h"
#include "Graphics/Objects/Mesh/Mesh.h"
#include "Graphics/Objects/GPU/Buffer.h"

#include "Graphics/Drawers/DebugDrawer.h"
#include "Graphics/Buffers/InstanceDataBuffer.h"
#include "Surface_VS.h"
#include "Unlit_PS.h"
#include "Line_PS.h"
#include "Line_VS.h"

#include "Outline_VS.h"
#include "Outline_PS.h"

constexpr size_t MAX_LINEVERTEX_PER_DRAWCALL = 1000;

bool DebugRenderPass::Initialize(RenderHardwareInterface& aRHI)
{
	myLineVertexBuffer = std::make_unique<Buffer>();

	if (!aRHI.CreateDynamicVertexBuffer("LineRenderer", sizeof(LineVertex), MAX_LINEVERTEX_PER_DRAWCALL, *myLineVertexBuffer))
	{
		SPDLOG_ERROR("Failed to create Line Render Buffer!");
		return false;
	}

	myInstanceBuffer = std::make_unique<Buffer>();

	unsigned bufferSize = sizeof(InstanceData) * MAX_MESH_INSTANCES;
	unsigned bufferStride = sizeof(InstanceData);

	if (!aRHI.CreateStructuredBuffer("InstanceBuffer", bufferSize, bufferStride, *myInstanceBuffer))
	{
		SPDLOG_ERROR("Failed to create Instance Buffer!");
		return false;
	}

	// Wireframe Mesh PSO
	{
		PipelineStateDescription wireframePSODesc;
		wireframePSODesc.Name = "Wireframe Mesh";
		wireframePSODesc.VertexShader.ByteCode = INTERNAL_Surface_VS_ByteCode;
		wireframePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Surface_VS_ByteCode);
		wireframePSODesc.PixelShader.ByteCode = INTERNAL_Outline_PS_ByteCode;
		wireframePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Outline_PS_ByteCode);
		wireframePSODesc.InputLayoutElements = Vertex::Description;
		wireframePSODesc.BlendMode = BlendMode::Alpha;
		wireframePSODesc.RasterizerState = RasterizerState::FillWireframeCullNone;

		myMeshWireframePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(wireframePSODesc, *myMeshWireframePSO))
		{
			SPDLOG_CRITICAL("Failed to create Wireframe pipeline state object!");
			return false;
		}
	}

	// Outline Mesh PSO
	{
		PipelineStateDescription outlinePSODesc;
		outlinePSODesc.Name = "Outline Mesh";
		outlinePSODesc.VertexShader.ByteCode = INTERNAL_Outline_VS_ByteCode;
		outlinePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Outline_VS_ByteCode);
		outlinePSODesc.PixelShader.ByteCode = INTERNAL_Outline_PS_ByteCode;
		outlinePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Outline_PS_ByteCode);
		outlinePSODesc.InputLayoutElements = Vertex::Description;
		outlinePSODesc.BlendMode = BlendMode::Alpha;
		outlinePSODesc.RasterizerState = RasterizerState::FillSolidCullFront;

		myMeshOutlinePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(outlinePSODesc, *myMeshOutlinePSO))
		{
			SPDLOG_CRITICAL("Failed to create Outline pipeline state object!");
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

	FrameBuffer frameBuffer{};
	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
	frameBuffer.Projection = aContext.Camera->RenderCamera.GetProjection();
	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();
	frameBuffer.TypeOfRenderPass = static_cast<unsigned int>(aContext.RenderSettings->DebugRenderMode);
	frameBuffer.ViewportSize = aContext.RHI->GetClientSize();

	aContext.Buffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

	RenderWireMeshes(inoutCommandList, aContext);
	RenderOutlines(inoutCommandList, aContext);
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

	inoutCommandList.SetPipelineStateObject(*myMeshWireframePSO);

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

			aContext.Buffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader);

			inoutCommandList.DrawIndexed(element.NumIndices, element.IndexOffset);
		}

		inoutCommandList.EndEvent();
	}
}

void DebugRenderPass::RenderLines(GraphicsCommandList& inoutCommandList, RenderContext& aContext)
{
	// TODO: Make it so that when a dynamic vertex buffer is full use a new one.

	const std::vector<LineDrawCall>& lineCalls = DebugDrawer::Get().GetLineDrawCalls();

	if (lineCalls.empty())
	{
		return;
	}

	inoutCommandList.SetPipelineStateObject(*myLinePSO);

	myLineVertices.clear();

	for (const LineDrawCall& call : lineCalls)
	{
		if (!aContext.Camera->RenderCamera.GetFrustum().IsInside(call.End) && 
			!aContext.Camera->RenderCamera.GetFrustum().IsInside(call.Start))
		{
			continue;
		}

		myLineVertices.push_back({ { call.Start.x, call.Start.y, call.Start.z, 1.f }, call.Color });
		myLineVertices.push_back({ { call.End.x,   call.End.y,   call.End.z,   1.f }, call.Color });
	}

	inoutCommandList.SetVertexBuffer(myLineVertexBuffer.get());

	for (size_t offset = 0; offset < myLineVertices.size(); offset += MAX_LINEVERTEX_PER_DRAWCALL)
	{
		const size_t count = std::min(MAX_LINEVERTEX_PER_DRAWCALL, myLineVertices.size() - offset);

		if (!inoutCommandList.UpdateDynamicVertexBuffer(*myLineVertexBuffer, myLineVertices.data() + offset, count * sizeof(LineVertex)))
		{
			return;
		}

		inoutCommandList.DrawSpecificVertices(static_cast<unsigned int>(count), 0);
	}
}

void DebugRenderPass::RenderOutlines(GraphicsCommandList& inoutCommandList, RenderContext& aContext)
{
	const std::vector<OutlineDrawCall>& outlineCalls = DebugDrawer::Get().GetMeshOutlineDrawCalls();

	if (outlineCalls.empty())
	{
		return;
	}

	inoutCommandList.SetPipelineStateObject(*myMeshWireframePSO);

	for (const auto& drawCall : outlineCalls)
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
			objectBuffer.Color = drawCall.Color;
			objectBuffer.IsInstanced = drawCall.Instances != nullptr;

			aContext.Buffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::ObjectBuffer, objectBuffer, 1, PipelineStage_VertexShader);

			if (objectBuffer.IsInstanced)
			{
				const unsigned int instanceCount = std::min(drawCall.AmountOfInstances, MAX_MESH_INSTANCES);

				if (instanceCount > MAX_MESH_INSTANCES)
				{
					SPDLOG_WARN("Too many instances submitted!");
					continue;
				}

				inoutCommandList.UpdateConstantBuffer(*myInstanceBuffer, drawCall.Instances, instanceCount * sizeof(InstanceData));

				inoutCommandList.SetShaderResource(myInstanceBuffer.get(), 20, PipelineStage_VertexShader);

				inoutCommandList.DrawIndexedInstanced(element.NumIndices, element.IndexOffset, instanceCount);

				inoutCommandList.UnbindShaderResource(20, PipelineStage_VertexShader);
			}
			else
			{
				inoutCommandList.DrawIndexed(element.NumIndices, element.IndexOffset);
			}

		}

		inoutCommandList.EndEvent();
	}
}
