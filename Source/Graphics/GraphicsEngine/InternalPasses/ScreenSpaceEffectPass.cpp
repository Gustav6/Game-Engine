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
#include "SceneLuminance_PS.h"

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

	// Scene Luminance PSO
	{
		PipelineStateDescription sceneLuminancePSODesc = {};
		sceneLuminancePSODesc.Name = "SceneLuminancePSO";
		sceneLuminancePSODesc.BlendMode = BlendMode::Opaque;
		sceneLuminancePSODesc.Topology = Topology::TriangleStrip;
		sceneLuminancePSODesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		sceneLuminancePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		sceneLuminancePSODesc.PixelShader.ByteCode = INTERNAL_SceneLuminance_PS_ByteCode;
		sceneLuminancePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_SceneLuminance_PS_ByteCode);

		mySceneLuminancePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(sceneLuminancePSODesc, *mySceneLuminancePSO))
		{
			GELOG(Error, "Failed to create Scene Luminance pipeline state object!");
		}
	}

	// Scene Luminance Texture
	{
		mySceneLuminance = std::make_unique<Texture>();

		CommonUtilities::Vector2ui clientSize = aRHI.GetClientSize();

		if (!aRHI.CreateTexture("SceneLuminance", clientSize.x, clientSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *mySceneLuminance))
		{
			GELOG(Error, "Failed to create Scene Luminance texture!");
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

	inoutCommandList.BeginEvent("Bloom Pass");
	{
		inoutCommandList.SetRenderTarget(mySceneLuminance.get(), nullptr);

		inoutCommandList.SetShaderResource(aHDRTexture, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*mySceneLuminancePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

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
