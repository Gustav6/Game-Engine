#include "GraphicsEngine.pch.h"
#include "DebugRenderPass.h"

#include "Shared/DrawCallSubmitter.h"

#include "GraphicsEngine/Core/RenderContext.h"
#include "GraphicsEngine/Core/SceneCamera.h"
#include "GraphicsEngine/Core/FrameTargets.h"
#include "GraphicsEngine/Core/RenderEnums.h"

#include "GraphicsEngine/RHI/RenderHardwareInterface.h"

#include "GraphicsEngine/ConstantBuffers/FrameBuffer.h"

#include "GraphicsEngine/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"
#include <GraphicsEngine/RHI/PipelineStateObject.h>

#include "GraphicsEngine/Objects/Scene/Camera.h"

#include "GraphicsEngine/Drawers/DebugDrawer.h"

#include "Surface_VS.h"
#include "PBL_PS.h"

#include <tracy/core/Tracy.hpp>

bool DebugRenderPass::Initialize(RenderHardwareInterface& aRHI)
{
	// Deffered Dir Light PSO
	{
		PipelineStateDescription debugDesc;
		debugDesc.Name = "Debug";
		debugDesc.VertexShader.ByteCode = INTERNAL_Surface_VS_ByteCode;
		debugDesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Surface_VS_ByteCode);
		debugDesc.PixelShader.ByteCode = INTERNAL_PBL_PS_ByteCode;
		debugDesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_PBL_PS_ByteCode);
		debugDesc.BlendMode = BlendMode::Opaque;
		debugDesc.RasterizerState = RasterizerState::FillWireframeCullNone;

		myDebugPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(debugDesc, *myDebugPSO))
		{
			GELOG(Error, "Failed to create debug pipeline state object!");
			return false;
		}
	}

	return true;
}

//void DebugRenderPass::Execute(GraphicsCommandList& inoutCommandList, FrameTargets& aFrameTargets, RenderContext& aContext)
//{
//	inoutCommandList.BeginEvent("Debug Render");
//
//	inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, aFrameTargets.DepthBuffer);
//
//	FrameBuffer frameBuffer;
//	frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
//	frameBuffer.Projection = aContext.Camera->RenderCamera.GetProjection();
//	frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();
//	frameBuffer.TypeOfRenderPass = static_cast<unsigned int>(aContext.Mode);
//
//	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);
//
//	//DrawCallSubmitter::RenderDrawCalls(inoutCommandList, aItems, aContext, DrawCallFlag_BindTextures | DrawCallFlag_BindMaterial);
//
//	inoutCommandList.EndEvent();
//}
