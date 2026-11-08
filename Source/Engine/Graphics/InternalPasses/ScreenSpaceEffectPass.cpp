#include "Graphics/GraphicsEngine.pch.h"
#include "ScreenSpaceEffectPass.h"

#include "Graphics/GraphicsEngine.hpp"

#include "Graphics/Objects/GPU/Texture.h"

#include "Graphics/ConstantBuffers/FrameBuffer.h"

#include "Graphics/RHI/GraphicsCommandList.h"
#include "Graphics/RHI/RenderHardwareInterface.h"
#include "Graphics/RHI/PipelineStateObject.h"

#include "Graphics/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "Graphics/Core/FrameTargets.h"
#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/RenderEnums.h"

#include "Graphics/InternalOperations/PostProcessFilters.h"

#include "Fullscreen_VS.h"
#include "Tonemap_PS.h"
#include "SceneLuminance_PS.h"
#include "Bloom_PS.h"
#include "BloomUpsample_PS.h"


bool ScreenSpaceEffectPass::Initialize(RenderHardwareInterface& aRHI)
{
	myBloomEnabled = true;

	// Tonemap PSO
	{
		PipelineStateDescription tonemapPSODesc = {};
		tonemapPSODesc.Name = "TonemapPSO";
		tonemapPSODesc.BlendMode = BlendMode::Opaque;
		tonemapPSODesc.Topology = Topology::TriangleStrip;
		tonemapPSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		tonemapPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		tonemapPSODesc.PixelShader.ByteCode = INTERNAL_Tonemap_PS_ByteCode;
		tonemapPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Tonemap_PS_ByteCode);

		myTonemapPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(tonemapPSODesc, *myTonemapPSO))
		{
			SPDLOG_CRITICAL("Failed to create Tonemap pipeline state object!");
			return false;
		}
	}

	// Scene Luminance PSO
	{
		PipelineStateDescription sceneLuminancePSODesc = {};
		sceneLuminancePSODesc.Name = "SceneLuminancePSO";
		sceneLuminancePSODesc.BlendMode = BlendMode::Opaque;
		sceneLuminancePSODesc.Topology = Topology::TriangleStrip;
		sceneLuminancePSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		sceneLuminancePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		sceneLuminancePSODesc.PixelShader.ByteCode = INTERNAL_SceneLuminance_PS_ByteCode;
		sceneLuminancePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_SceneLuminance_PS_ByteCode);

		myLuminancePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(sceneLuminancePSODesc, *myLuminancePSO))
		{
			SPDLOG_CRITICAL("Failed to create Scene Luminance pipeline state object!");
			return false;
		}
	}

	// Bloom PSO
	{
		PipelineStateDescription bloomPSODesc = {};
		bloomPSODesc.Name = "BloomPSO";
		bloomPSODesc.BlendMode = BlendMode::Additive;
		bloomPSODesc.Topology = Topology::TriangleStrip;
		bloomPSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		bloomPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		bloomPSODesc.PixelShader.ByteCode = INTERNAL_Bloom_PS_ByteCode;
		bloomPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Bloom_PS_ByteCode);

		myBloomPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(bloomPSODesc, *myBloomPSO))
		{
			SPDLOG_CRITICAL("Failed to create Bloom pipeline state object!");
			return false;
		}
	}

	// Scene Luminance Texture
	{
		mySceneLuminance = std::make_unique<Texture>();

		CommonUtilities::Vector2ui clientSize = aRHI.GetClientSize();

		if (!aRHI.CreateTexture("SceneLuminance", clientSize.x, clientSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *mySceneLuminance))
		{
			SPDLOG_CRITICAL("Failed to create Scene Luminance texture!");
			return false;
		}
	}

	// Bloom Mips
	{
		CommonUtilities::Vector2ui mipSize = aRHI.GetClientSize() / 2u;

		for (int i = 0; i < myBloomMips.size(); ++i)
		{
			myBloomMips[i].Size = mipSize;

			std::string mipName = std::format("BloomMip {}", i + 1);

			myBloomMips[i].Texture = std::make_unique<Texture>();

			if (!aRHI.CreateTexture(mipName, mipSize.x, mipSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myBloomMips[i].Texture))
			{
				SPDLOG_CRITICAL("Failed to create Bloom Mip texture!");
				return false;
			}

			mipSize.x = std::max(1u, mipSize.x / 2);
			mipSize.y = std::max(1u, mipSize.y / 2);
		}
	}

	return true;
}

void ScreenSpaceEffectPass::Execute(
	GraphicsCommandList& inoutCommandList,
	FrameTargets& aFrameTargets,
	RenderContext& aContext,
	Texture* aHDRTexture)
{
	inoutCommandList.BeginEvent("Screen Space Effects");

	if (myBloomEnabled)
	{
		BloomPass(inoutCommandList, aContext, aHDRTexture);
	}

	inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, nullptr);

	TonemapPass(inoutCommandList, aContext, aHDRTexture);

	inoutCommandList.EndEvent();
}

void ScreenSpaceEffectPass::SetBloomStatus(bool aEnabled)
{
	myBloomEnabled = aEnabled;
}

void ScreenSpaceEffectPass::BloomPass(
	GraphicsCommandList& inoutCommandList,
	RenderContext& aContext,
	Texture* aHDRTexture)
{
	if (aContext.RenderSettings->DebugRenderMode != DebugRenderMode::None)
	{
		return;
	}

	inoutCommandList.BeginEvent("Bloom Pass");

	inoutCommandList.BeginEvent("Scene Luminance");
	{
		inoutCommandList.SetRenderTarget(mySceneLuminance.get(), nullptr);

		inoutCommandList.SetShaderResource(aHDRTexture, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myLuminancePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

	aContext.PostProcessFilters->BloomDownsample(
		inoutCommandList,
		*mySceneLuminance,
		*myBloomMips[0].Texture);

	for (size_t i = 1; i < myBloomMips.size(); ++i)
	{
		aContext.PostProcessFilters->BloomDownsample(
			inoutCommandList,
			*myBloomMips[i - 1].Texture,
			*myBloomMips[i].Texture);
	}

	for (int i = static_cast<int>(myBloomMips.size()) - 1; i > 0; --i)
	{
		const BloomMip& mip = myBloomMips[i];
		const BloomMip& nextMip = myBloomMips[i - 1];

		inoutCommandList.UnbindShaderResource(0, PipelineStage_PixelShader);

		aContext.PostProcessFilters->BloomAdditiveUpsample(
			inoutCommandList,
			*mip.Texture,
			*nextMip.Texture);
	}

	inoutCommandList.BeginEvent("Apply Bloom");
	{
		inoutCommandList.SetRenderTarget(aHDRTexture, nullptr);

		inoutCommandList.SetShaderResource(myBloomMips[0].Texture.get(), 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myBloomPSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

	inoutCommandList.EndEvent();
}

void ScreenSpaceEffectPass::TonemapPass(
	GraphicsCommandList& inoutCommandList,
	RenderContext& aContext,
	Texture* aHDRTexture)
{
	inoutCommandList.BeginEvent("Tonemap Pass");

	FrameBuffer frameBuffer;

	if (aContext.RenderSettings->DebugRenderMode == DebugRenderMode::None)
	{
		frameBuffer.TypeOfTonemap = static_cast<unsigned int>(aContext.RenderSettings->ToneMapper);
	}
	else
	{
		frameBuffer.TypeOfTonemap = static_cast<unsigned int>(ToneMapper::None);
	}

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_PixelShader);

	inoutCommandList.SetShaderResource(aHDRTexture, 0, PipelineStage_PixelShader);
	inoutCommandList.SetPipelineStateObject(*myTonemapPSO);
	inoutCommandList.Draw(4);

	inoutCommandList.EndEvent();
}
