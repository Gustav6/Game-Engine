#include "Graphics/GraphicsEngine.pch.h"
#include "PixelPickingPass.h"

#include "Shared/DrawCallSubmitter.h"

#include "Graphics/Core/RenderItem.h"
#include "Graphics/Core/RenderContext.h"
#include "Graphics/Core/RenderEnums.h"
#include "Graphics/Core/SceneCamera.h"

#include "Graphics/ConstantBuffers/FrameBuffer.h"
#include "Graphics/ConstantBuffers/Manager/ConstantBufferManager.h"

#include "Graphics/Objects/GPU/Texture.h"
#include "Graphics/Objects/Scene/Camera.h"

#include "Graphics/RHI/GraphicsCommandList.h"
#include "Graphics/RHI/RenderHardwareInterface.h"
#include "Graphics/RHI/PipelineStateObject.h"

#include "Wrappers/Logger/LoggerWrapper.h"

#include "PixelPicking_PS.h"

PixelPickingPass::~PixelPickingPass() = default;


bool PixelPickingPass::Initialize(RenderHardwareInterface& aRHI)
{
    const CommonUtilities::Vector2ui resolution = aRHI.GetClientSize();

    {
        PipelineStateDescription pickingPSODesc{};

        pickingPSODesc.Name = "PixelPickingPSO";
        pickingPSODesc.BlendMode = BlendMode::Opaque;
        pickingPSODesc.Topology = Topology::TriangleList;
        pickingPSODesc.PixelShader.ByteCode = INTERNAL_PixelPicking_PS_ByteCode;
        pickingPSODesc.PixelShader.ByteCodeSize = sizeof(INTERNAL_PixelPicking_PS_ByteCode);

        myPickingPSO = std::make_unique<PipelineStateObject>();

        if (!aRHI.CreatePipelineStateObject(pickingPSODesc, *myPickingPSO))
        {
            SPDLOG_ERROR("Failed to create Pixel Picking pipeline state object!");
            return false;
        }
    }

    {
        myIDBuffer = std::make_unique<Texture>();

        if (!aRHI.CreateTexture("IDBuffer", resolution.x, resolution.y, 1, ResourceFormat::R32_UINT, ResourceBindFlag_RenderTarget, *myIDBuffer))
        {
            SPDLOG_ERROR("Failed to create IDBuffer ({}x{})", resolution.x, resolution.y);
            return false;
        }
    }

    {
        myIDStagingBuffer = std::make_unique<Texture>();

        if (!aRHI.CreateTexture("IDStagingBuffer", 1, 1, 1, ResourceFormat::R32_UINT, ResourceBindFlag_Staging, *myIDStagingBuffer))
        {
            SPDLOG_ERROR("Failed to create IDStagingBuffer!");
            return false;
        }
    }

    return true;
}

void PixelPickingPass::Execute(GraphicsCommandList& inoutCommandList, const std::vector<RenderItem>& aItems, Texture& aDepthBuffer, RenderContext& aContext)
{
    inoutCommandList.BeginEvent("Pixel Picking Pass");

    inoutCommandList.ClearRenderTarget(*myIDBuffer);

    FrameBuffer frameBuffer{};
    frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
    frameBuffer.Projection = aContext.Camera->RenderCamera.GetProjection();
    frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();

    aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

    inoutCommandList.SetRenderTarget(myIDBuffer.get(), &aDepthBuffer);
    inoutCommandList.SetOverridePipelineState(*myPickingPSO, PipelineStage_PixelShader);

    DrawCallSubmitter::RenderDrawCalls(inoutCommandList, aItems, aContext, DrawCallFlag_BindMaterial);

    inoutCommandList.ClearOverridePipelineState();
    inoutCommandList.EndEvent();
}

const Texture* PixelPickingPass::GetIDBuffer() const
{
    return myIDBuffer.get();
}

const Texture* PixelPickingPass::GetIDStagingBuffer() const
{
    return myIDStagingBuffer.get();
}