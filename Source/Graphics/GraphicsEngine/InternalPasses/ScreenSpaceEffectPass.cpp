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

#include "Fullscreen_VS.h"
#include "Tonemap_PS.h"
#include "SceneLuminance_PS.h"
#include "Bloom_PS.h"

#include "Core/RenderContext.h"

#include <tracy/core/Tracy.hpp>

#include "InternalOperations/PostProcessFilters.h"

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
		bloomPSODesc.BlendMode = BlendMode::Alpha;
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

	// Fullscreen Buffer Texture
	{
		myResampleFullSize = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize();

		if (!aRHI.CreateTexture("FullscreenBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myResampleFullSize))
		{
			SPDLOG_CRITICAL("Failed to create Fullscreen Buffer texture!");
			return false;
		}
	}

	// Half Screen Buffer Texture
	{
		myResampleHalfSize = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize() / 2u;

		if (!aRHI.CreateTexture("HalfScreenBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myResampleHalfSize))
		{
			SPDLOG_CRITICAL("Failed to create Half Screen Buffer texture!");
			return false;
		}
	}

	// Quarter Screen Buffer Texture
	{
		myResampleQuarterSize1 = std::make_unique<Texture>();
		myResampleQuarterSize2 = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize() / 4u;

		if (!aRHI.CreateTexture("QuarterScreenBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myResampleQuarterSize1))
		{
			SPDLOG_CRITICAL("Failed to create Quarter Screen Buffer texture!");
			return false;
		}
		if (!aRHI.CreateTexture("QuarterScreenBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myResampleQuarterSize2))
		{
			SPDLOG_CRITICAL("Failed to create Quarter Screen Buffer texture!");
			return false;
		}
	}

	// Quarter Screen Buffer Texture with blur
	{
		myBlurredTexture = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize() / 4u;

		if (!aRHI.CreateTexture("QuarterBlurred", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myBlurredTexture))
		{
			SPDLOG_CRITICAL("Failed to create Quarter Blurred Screen Buffer texture!");
			return false;
		}
	}

	// Scene Bloom Buffer Texture
	{
		myBloomTexture = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize();

		if (!aRHI.CreateTexture("BloomBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myBloomTexture))
		{
			SPDLOG_CRITICAL("Failed to create Bloom Buffer texture!");
			return false;
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

	SSAOPass();

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

void ScreenSpaceEffectPass::SSAOPass()
{
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

	aContext.PostProcessFilters->Resample(inoutCommandList, *mySceneLuminance, *myResampleHalfSize);
	aContext.PostProcessFilters->Resample(inoutCommandList, *myResampleHalfSize, *myResampleQuarterSize1);

	aContext.PostProcessFilters->BlurHorizontal(inoutCommandList, *myResampleQuarterSize1, *myBlurredTexture);
	aContext.PostProcessFilters->BlurVertical(inoutCommandList, *myBlurredTexture, *myResampleQuarterSize2);
	aContext.PostProcessFilters->BlurHorizontal(inoutCommandList, *myResampleQuarterSize2, *myResampleQuarterSize1);
	aContext.PostProcessFilters->BlurVertical(inoutCommandList, *myResampleQuarterSize1, *myBlurredTexture);

	aContext.PostProcessFilters->Resample(inoutCommandList, *myBlurredTexture, *myResampleHalfSize);
	aContext.PostProcessFilters->Resample(inoutCommandList, *myResampleHalfSize, *myBloomTexture);

	inoutCommandList.BeginEvent("Apply Bloom");
	{
		inoutCommandList.SetRenderTarget(aHDRTexture, nullptr);

		inoutCommandList.SetShaderResource(myBloomTexture.get(), 0, PipelineStage_PixelShader);
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
