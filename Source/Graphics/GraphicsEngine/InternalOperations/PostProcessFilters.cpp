#include "GraphicsEngine.pch.h"
#include "PostProcessFilters.h"

#include "RHI/PipelineStateObject.h"
#include "RHI/RenderHardwareInterface.h"
#include "RHI/GraphicsCommandList.h"

#include <Fullscreen_VS.h>
#include <GaussianBlurH_PS.h>
#include <GaussianBlurV_PS.h>
#include <BloomDownsample_PS.h>
#include <BloomUpsample_PS.h>

bool PostProcessFilters::Initialize(RenderHardwareInterface& aRHI)
{
	// Downsample PSO
	{
		PipelineStateDescription downsamplePSODesc = {};
		downsamplePSODesc.Name = "DownsamplePSO";
		downsamplePSODesc.BlendMode = BlendMode::Opaque;
		downsamplePSODesc.Topology = Topology::TriangleStrip;
		downsamplePSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		downsamplePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		downsamplePSODesc.PixelShader.ByteCode = INTERNAL_BloomDownsample_PS_ByteCode;
		downsamplePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_BloomDownsample_PS_ByteCode);

		myDownsamplePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(downsamplePSODesc, *myDownsamplePSO))
		{
			SPDLOG_CRITICAL("Failed to create Downsample pipeline state object!");
			return false;
		}
	}

	// Upsample PSO
	{
		PipelineStateDescription upsamplePSODesc = {};
		upsamplePSODesc.Name = "UpsamplePSO";
		upsamplePSODesc.BlendMode = BlendMode::Additive;
		upsamplePSODesc.Topology = Topology::TriangleStrip;
		upsamplePSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		upsamplePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		upsamplePSODesc.PixelShader.ByteCode = INTERNAL_BloomUpsample_PS_ByteCode;
		upsamplePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_BloomUpsample_PS_ByteCode);

		myAdditiveUpsamplePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(upsamplePSODesc, *myAdditiveUpsamplePSO))
		{
			SPDLOG_CRITICAL("Failed to create Upsample pipeline state object!");
			return false;
		}
	}

	// Gaussian blur horizontal PSO
	{
		PipelineStateDescription gaussianBlurPSODesc = {};
		gaussianBlurPSODesc.Name = "HorizontalBlurPSO";
		gaussianBlurPSODesc.BlendMode = BlendMode::Opaque;
		gaussianBlurPSODesc.Topology = Topology::TriangleStrip;
		gaussianBlurPSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		gaussianBlurPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		gaussianBlurPSODesc.PixelShader.ByteCode = INTERNAL_GaussianBlurH_PS_ByteCode;
		gaussianBlurPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_GaussianBlurH_PS_ByteCode);

		myBlurHorizontalPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(gaussianBlurPSODesc, *myBlurHorizontalPSO))
		{
			SPDLOG_CRITICAL("Failed to create Horizontal blur pipeline state object!");
			return false;
		}
	}

	// Gaussian blur vertical PSO
	{
		PipelineStateDescription gaussianBlurPSODesc = {};
		gaussianBlurPSODesc.Name = "VerticalBlurPSO";
		gaussianBlurPSODesc.BlendMode = BlendMode::Opaque;
		gaussianBlurPSODesc.Topology = Topology::TriangleStrip;
		gaussianBlurPSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		gaussianBlurPSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		gaussianBlurPSODesc.PixelShader.ByteCode = INTERNAL_GaussianBlurV_PS_ByteCode;
		gaussianBlurPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_GaussianBlurV_PS_ByteCode);

		myBlurVerticalPSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(gaussianBlurPSODesc, *myBlurVerticalPSO))
		{
			SPDLOG_CRITICAL("Failed to create Vertical blur pipeline state object!");
			return false;
		}
	}

	return true;
}

void PostProcessFilters::Downsample(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const
{
	inoutCommandList.BeginEvent("Downsample");
	{
		inoutCommandList.SetRenderTarget(&aDestination, nullptr);

		inoutCommandList.SetShaderResource(&aSource, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myDownsamplePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();
}

void PostProcessFilters::AdditiveUpsample(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const
{
	inoutCommandList.BeginEvent("Upsample");
	{
		inoutCommandList.SetRenderTarget(&aDestination, nullptr);

		inoutCommandList.SetShaderResource(&aSource, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myAdditiveUpsamplePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();
}

void PostProcessFilters::BlurHorizontal(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const
{
	inoutCommandList.BeginEvent("Horizontal Blur");
	{
		inoutCommandList.SetRenderTarget(&aDestination, nullptr);

		inoutCommandList.SetPipelineStateObject(*myBlurHorizontalPSO);
		inoutCommandList.SetShaderResource(&aSource, 0, PipelineStage_PixelShader);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();
}

void PostProcessFilters::BlurVertical(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const
{
	inoutCommandList.BeginEvent("Vertical Blur");
	{
		inoutCommandList.SetRenderTarget(&aDestination, nullptr);

		inoutCommandList.SetPipelineStateObject(*myBlurVerticalPSO);
		inoutCommandList.SetShaderResource(&aSource, 0, PipelineStage_PixelShader);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();
}
