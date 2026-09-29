#include "GraphicsEngine.pch.h"
#include "ScreenSpaceEffectPass.h"

#include "GraphicsEngine/Core/RenderEnums.h"

#include "GraphicsEngine/Objects/GPU/Texture.h"

#include "GraphicsEngine/ConstantBuffers/FrameBuffer.h"

#include "GraphicsEngine/RHI/GraphicsCommandList.h"
#include "GraphicsEngine/RHI/RenderHardwareInterface.h"
#include "GraphicsEngine/RHI/PipelineStateObject.h"

#include "GraphicsEngine/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "GraphicsEngine/Core/FrameTargets.h"

#include "Logger/Logger.h"

#include "FullTexture_VS.h"
#include "Tonemap_PS.h"

#include "Core/RenderContext.h"

ScreenSpaceEffectPass::ScreenSpaceEffectPass(RenderHardwareInterface& aRHI)
{
	// Tonemap PSO
	{
		PipelineStateDescription tonemapPSODesc = {};
		tonemapPSODesc.Name = "TonemapPSO";
		tonemapPSODesc.BlendMode = BlendMode::Opaque;
		tonemapPSODesc.Topology = Topology::TriangleStrip;
		tonemapPSODesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		tonemapPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		tonemapPSODesc.PixelShader.ByteCode = INTERNAL_Tonemap_PS_ByteCode;
		tonemapPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Tonemap_PS_ByteCode);

		myTonemapPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(tonemapPSODesc, *myTonemapPSO))
		{
			GELOG(Error, "Failed to create Tonemap pipeline state object!");
		}
	}
}

void ScreenSpaceEffectPass::Execute(
	GraphicsCommandList& inoutCommandList, 
	FrameTargets& aFrameTargets,
	RenderContext& aContext, 
	Texture* aHDRTexture)
{
	inoutCommandList.BeginEvent("Screen Space Effects");

	inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, nullptr);

	inoutCommandList.BeginEvent("Tonemap Pass");
	{
		FrameBuffer frameBuffer;
		frameBuffer.TypeOfTonemap = static_cast<unsigned int>(aContext.Tonmap);

		aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_PixelShader);

		inoutCommandList.SetShaderResource(aHDRTexture, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myTonemapPSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

	inoutCommandList.EndEvent();
}
