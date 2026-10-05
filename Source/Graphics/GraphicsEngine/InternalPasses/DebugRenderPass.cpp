#include "GraphicsEngine.pch.h"
#include "DebugRenderPass.h"

#include "Shared/DrawCallSubmitter.h"

#include "GraphicsEngine/Core/RenderContext.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

#include "GraphicsEngine/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"
#include <GraphicsEngine/RHI/PipelineStateObject.h>

#include "GraphicsEngine/Drawers/DebugDrawer.h"

#include "Surface_VS.h"
#include "PBL_PS.h"

#include <tracy/core/Tracy.hpp>

//bool ForwardRenderPass::Initialize(RenderHardwareInterface& aRHI)
//{
//	// Deffered Dir Light PSO
//	{
//		PipelineStateDescription debugDesc;
//		debugDesc.Name = "Debug";
//		debugDesc.VertexShader.ByteCode = INTERNAL_Surface_VS_ByteCode;
//		debugDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Surface_VS_ByteCode);
//		debugDesc.PixelShader.ByteCode = INTERNAL_PBL_PS_ByteCode;
//		debugDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_PBL_PS_ByteCode);
//		debugDesc.BlendMode = BlendMode::Opaque;
//
//		myDebugPSO = std::make_unique<PipelineStateObject>();
//
//		if (!aRHI.CreatePipelineStateObject(debugDesc, *myDebugPSO))
//		{
//			GELOG(Error, "Failed to create debug pipeline state object!");
//			return false;
//		}
//	}
//}
//
//void ForwardRenderPass::Execute(GraphicsCommandList& inoutCommandList, FrameTargets& aFrameTargets, RenderContext& aContext)
//{
//}
