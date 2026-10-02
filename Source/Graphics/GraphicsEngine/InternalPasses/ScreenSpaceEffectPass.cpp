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
#include "Resample_PS.h"
#include "Bloom_PS.h"
#include "GaussianV_PS.h"
#include "GaussianH_PS.h"

#include "Core/RenderContext.h"

#include <tracy/core/Tracy.hpp>

ScreenSpaceEffectPass::ScreenSpaceEffectPass(RenderHardwareInterface& aRHI)
{
	myBloomEnabled = true;

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

	// Resample PSO
	{
		PipelineStateDescription resamplePSODesc = {};
		resamplePSODesc.Name = "ResamplePSO";
		resamplePSODesc.BlendMode = BlendMode::Opaque;
		resamplePSODesc.Topology = Topology::TriangleStrip;
		resamplePSODesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		resamplePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		resamplePSODesc.PixelShader.ByteCode = INTERNAL_Resample_PS_ByteCode;
		resamplePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Resample_PS_ByteCode);

		myResamplePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(resamplePSODesc, *myResamplePSO))
		{
			GELOG(Error, "Failed to create Resample pipeline state object!");
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

		myLuminancePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(sceneLuminancePSODesc, *myLuminancePSO))
		{
			GELOG(Error, "Failed to create Scene Luminance pipeline state object!");
		}
	}

	// Gaussian blur horizontal PSO
	{
		PipelineStateDescription gaussianBlurPSODesc = {};
		gaussianBlurPSODesc.Name = "GaussianBlurPSO";
		gaussianBlurPSODesc.BlendMode = BlendMode::Opaque;
		gaussianBlurPSODesc.Topology = Topology::TriangleStrip;
		gaussianBlurPSODesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		gaussianBlurPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		gaussianBlurPSODesc.PixelShader.ByteCode = INTERNAL_GaussianH_PS_ByteCode;
		gaussianBlurPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_GaussianH_PS_ByteCode);

		myGaussianBlurHorizontalPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(gaussianBlurPSODesc, *myGaussianBlurHorizontalPSO))
		{
			GELOG(Error, "Failed to create Gaussian blur pipeline state object!");
		}
	}

	// Gaussian blur vertical PSO
	{
		PipelineStateDescription gaussianBlurPSODesc = {};
		gaussianBlurPSODesc.Name = "GaussianBlurPSO";
		gaussianBlurPSODesc.BlendMode = BlendMode::Opaque;
		gaussianBlurPSODesc.Topology = Topology::TriangleStrip;
		gaussianBlurPSODesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		gaussianBlurPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		gaussianBlurPSODesc.PixelShader.ByteCode = INTERNAL_GaussianV_PS_ByteCode;
		gaussianBlurPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_GaussianV_PS_ByteCode);

		myGaussianBlurVerticalPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(gaussianBlurPSODesc, *myGaussianBlurVerticalPSO))
		{
			GELOG(Error, "Failed to create Gaussian blur pipeline state object!");
		}
	}

	// Bloom PSO
	{
		PipelineStateDescription bloomPSODesc = {};
		bloomPSODesc.Name = "BloomPSO";
		bloomPSODesc.BlendMode = BlendMode::Alpha;
		bloomPSODesc.Topology = Topology::TriangleStrip;
		bloomPSODesc.VertexShader.ByteCode = INTERNAL_FullTexture_VS_ByteCode;
		bloomPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_FullTexture_VS_ByteCode);
		bloomPSODesc.PixelShader.ByteCode = INTERNAL_Bloom_PS_ByteCode;
		bloomPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Bloom_PS_ByteCode);

		myBloomPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(bloomPSODesc, *myBloomPSO))
		{
			GELOG(Error, "Failed to create Bloom pipeline state object!");
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

	// Fullscreen Buffer Texture
	{
		myResampleFullSize = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize();

		if (!aRHI.CreateTexture("FullscreenBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myResampleFullSize))
		{
			GELOG(Error, "Failed to create Fullscreen Buffer texture!");
		}
	}

	// Half Screen Buffer Texture
	{
		myResampleHalfSize = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize() / 2u;

		if (!aRHI.CreateTexture("HalfScreenBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myResampleHalfSize))
		{
			GELOG(Error, "Failed to create Half Screen Buffer texture!");
		}
	}

	// Quarter Screen Buffer Texture
	{
		myResampleQuarterSize = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize() / 4u;

		if (!aRHI.CreateTexture("QuarterScreenBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myResampleQuarterSize))
		{
			GELOG(Error, "Failed to create Quarter Screen Buffer texture!");
		}
	}

	// Quarter Screen Buffer Texture with blur
	{
		myBlurredTexture = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize() / 4u;

		if (!aRHI.CreateTexture("QuarterBlurred", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myBlurredTexture))
		{
			GELOG(Error, "Failed to create Quarter Blurred Screen Buffer texture!");
		}
	}

	// Scene Bloom Buffer Texture
	{
		myBloomTexture = std::make_unique<Texture>();

		CommonUtilities::Vector2ui bufferSize = aRHI.GetClientSize();

		if (!aRHI.CreateTexture("BloomBuffer", bufferSize.x, bufferSize.y, 1, ResourceFormat::R16G16B16A16_FLOAT, ResourceBindFlag_ShaderResource | ResourceBindFlag_RenderTarget, *myBloomTexture))
		{
			GELOG(Error, "Failed to create Bloom Buffer texture!");
		}
	}
}

void ScreenSpaceEffectPass::Execute(
	GraphicsCommandList& inoutCommandList, 
	FrameTargets& aFrameTargets,
	RenderContext& aContext, 
	Texture* aHDRTexture)
{
	ZoneScopedN("Screen Space Effects Pass");

	inoutCommandList.BeginEvent("Screen Space Effects");

	if (myBloomEnabled)
	{
		BloomPass(inoutCommandList, aHDRTexture);
	}

	inoutCommandList.SetRenderTarget(aFrameTargets.RenderTarget, nullptr);

	TonemapPass(inoutCommandList, aContext, aHDRTexture);

	inoutCommandList.EndEvent();
}

void ScreenSpaceEffectPass::ToggleBloom()
{
	myBloomEnabled = !myBloomEnabled;
}

void ScreenSpaceEffectPass::BloomPass(
	GraphicsCommandList& inoutCommandList,
	Texture* aHDRTexture)
{
	inoutCommandList.BeginEvent("Bloom Pass");

	inoutCommandList.BeginEvent("Scene Luminance");
	{
		inoutCommandList.SetRenderTarget(mySceneLuminance.get(), nullptr);

		inoutCommandList.SetShaderResource(aHDRTexture, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myLuminancePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

	inoutCommandList.BeginEvent("Downsample Half Size");
	{
		inoutCommandList.SetRenderTarget(myResampleHalfSize.get(), nullptr);

		inoutCommandList.SetShaderResource(mySceneLuminance.get(), 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myResamplePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

	inoutCommandList.BeginEvent("Downsample Quarter Size");
	{
		inoutCommandList.SetRenderTarget(myBlurredTexture.get(), nullptr);

		inoutCommandList.SetShaderResource(myResampleHalfSize.get(), 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myResamplePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

	inoutCommandList.BeginEvent("Gaussian Blur");
	{
		inoutCommandList.BeginEvent("Horizontal Blur");
		{
			inoutCommandList.SetRenderTarget(myResampleQuarterSize.get(), nullptr);

			inoutCommandList.SetShaderResource(myBlurredTexture.get(), 0, PipelineStage_PixelShader);
			inoutCommandList.SetPipelineStateObject(*myGaussianBlurHorizontalPSO);
			inoutCommandList.Draw(4);
		}
		inoutCommandList.EndEvent();

		inoutCommandList.BeginEvent("Vertical Blur");
		{
			inoutCommandList.SetRenderTarget(myBlurredTexture.get(), nullptr);

			inoutCommandList.SetShaderResource(myResampleQuarterSize.get(), 0, PipelineStage_PixelShader);
			inoutCommandList.SetPipelineStateObject(*myGaussianBlurVerticalPSO);
			inoutCommandList.Draw(4);
		}
		inoutCommandList.EndEvent();

		inoutCommandList.BeginEvent("Horizontal Blur");
		{
			inoutCommandList.SetRenderTarget(myResampleQuarterSize.get(), nullptr);

			inoutCommandList.SetShaderResource(myBlurredTexture.get(), 0, PipelineStage_PixelShader);
			inoutCommandList.SetPipelineStateObject(*myGaussianBlurHorizontalPSO);
			inoutCommandList.Draw(4);
		}
		inoutCommandList.EndEvent();

		inoutCommandList.BeginEvent("Vertical Blur");
		{
			inoutCommandList.SetRenderTarget(myBlurredTexture.get(), nullptr);

			inoutCommandList.SetShaderResource(myResampleQuarterSize.get(), 0, PipelineStage_PixelShader);
			inoutCommandList.SetPipelineStateObject(*myGaussianBlurVerticalPSO);
			inoutCommandList.Draw(4);
		}
		inoutCommandList.EndEvent();
	}
	inoutCommandList.EndEvent();

	inoutCommandList.BeginEvent("Horizontal Half Size");
	{
		inoutCommandList.SetRenderTarget(myResampleHalfSize.get(), nullptr);

		inoutCommandList.SetShaderResource(myBlurredTexture.get(), 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myResamplePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

	inoutCommandList.BeginEvent("Horizontal Full Size");
	{
		inoutCommandList.SetRenderTarget(myBloomTexture.get(), nullptr);

		inoutCommandList.SetShaderResource(myResampleHalfSize.get(), 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myResamplePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();

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
	frameBuffer.TypeOfTonemap = static_cast<unsigned int>(aContext.Tonmap);

	aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_PixelShader);

	inoutCommandList.SetShaderResource(aHDRTexture, 0, PipelineStage_PixelShader);
	inoutCommandList.SetPipelineStateObject(*myTonemapPSO);
	inoutCommandList.Draw(4);

	inoutCommandList.EndEvent();
}
