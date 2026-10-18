#include "GraphicsEngine.pch.h"
#include "PostProcessFilters.h"

#include "RHI/PipelineStateObject.h"
#include "RHI/RenderHardwareInterface.h"
#include "RHI/GraphicsCommandList.h"

#include <Fullscreen_VS.h>
#include <GaussianBlurH_PS.h>
#include <GaussianBlurV_PS.h>
#include <Resample_PS.h>

bool PostProcessFilters::Initialize(RenderHardwareInterface& aRHI)
{
	// Resample PSO
	{
		PipelineStateDescription resamplePSODesc = {};
		resamplePSODesc.Name = "ResamplePSO";
		resamplePSODesc.BlendMode = BlendMode::Opaque;
		resamplePSODesc.Topology = Topology::TriangleStrip;
		resamplePSODesc.VertexShader.ByteCode = INTERNAL_Fullscreen_VS_ByteCode;
		resamplePSODesc.VertexShader.ByteCodeSize = sizeof(INTERNAL_Fullscreen_VS_ByteCode);
		resamplePSODesc.PixelShader.ByteCode = INTERNAL_Resample_PS_ByteCode;
		resamplePSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_Resample_PS_ByteCode);

		myResamplePSO = std::make_unique<PipelineStateObject>();

		if (!aRHI.CreatePipelineStateObject(resamplePSODesc, *myResamplePSO))
		{
			SPDLOG_CRITICAL("Failed to create Resample pipeline state object!");
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

void PostProcessFilters::Resample(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const
{
	// TODO: Check if it's an upsample or downsample here

	inoutCommandList.BeginEvent("Resample");
	{
		inoutCommandList.SetRenderTarget(&aDestination, nullptr);

		inoutCommandList.SetShaderResource(&aSource, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myResamplePSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();
}

void PostProcessFilters::BlurHorizontal(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const
{
	inoutCommandList.BeginEvent("Horizontal Blur");
	{
		inoutCommandList.SetRenderTarget(&aDestination, nullptr);

		inoutCommandList.SetShaderResource(&aSource, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myBlurHorizontalPSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();
}

void PostProcessFilters::BlurVertical(GraphicsCommandList& inoutCommandList, Texture& aSource, Texture& aDestination) const
{
	inoutCommandList.BeginEvent("Vertical Blur");
	{
		inoutCommandList.SetRenderTarget(&aDestination, nullptr);

		inoutCommandList.SetShaderResource(&aSource, 0, PipelineStage_PixelShader);
		inoutCommandList.SetPipelineStateObject(*myBlurVerticalPSO);
		inoutCommandList.Draw(4);
	}
	inoutCommandList.EndEvent();
}
