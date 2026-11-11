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
            SPDLOG_ERROR("Failed to create IDBuffer texture!");
            return false;
        }
    }

    {
        myIDStagingBuffer = std::make_unique<Texture>();

        if (!aRHI.CreateTexture("IDStagingBuffer", 1, 1, 1, ResourceFormat::R32_UINT, ResourceBindFlag_Staging, *myIDStagingBuffer))
        {
            SPDLOG_ERROR("Failed to create IDStagingBuffer texture!");
            return false;
        }
    }

	{
        myPickDepth = std::make_unique<Texture>();

        if (!aRHI.CreateDepthStencil("PickDepth", resolution.x, resolution.y, *myPickDepth, false))
        {
            SPDLOG_ERROR("Failed to create PickDepth texture!");
            return false;
        }
	}

    return true;
}

void PixelPickingPass::Execute(
    GraphicsCommandList& inoutCommandList, 
    const std::vector<RenderItem>& aItems, 
    RenderContext& aContext)
{
    if (!myPendingPick.has_value())
    {
        return;
    }

    inoutCommandList.BeginEvent("Pixel Picking Pass");

    inoutCommandList.ClearRenderTarget(*myIDBuffer);
	inoutCommandList.ClearDepthStencil(*myPickDepth);

    FrameBuffer frameBuffer{};
    frameBuffer.View = aContext.Camera->Transform.GetFastInverse();
    frameBuffer.Projection = aContext.Camera->RenderCamera.GetProjection();
    frameBuffer.CameraPosition = aContext.Camera->Transform.GetTranslation();

    aContext.ConstantBuffers->UpdateAndSetConstantBuffer(inoutCommandList, ConstantBuffer::FrameBuffer, frameBuffer, 0, PipelineStage_VertexShader | PipelineStage_PixelShader);

    inoutCommandList.SetRenderTarget(myIDBuffer.get(), myPickDepth.get());
    inoutCommandList.SetOverridePipelineState(*myPickingPSO, PipelineStage_PixelShader);

    DrawCallSubmitter::RenderDrawCalls(inoutCommandList, aItems, aContext, DrawCallFlag_BindMaterial);

    inoutCommandList.ClearOverridePipelineState();

    const unsigned x = std::min(myPendingPick->x, static_cast<unsigned>(myIDBuffer->GetViewport().Width) - 1);
    const unsigned y = std::min(myPendingPick->y, static_cast<unsigned>(myIDBuffer->GetViewport().Height) - 1);
    inoutCommandList.CopyRegion(*myIDBuffer, *myIDStagingBuffer, x, y, 1, 1);

    myPendingPick.reset();
    myHasAPickResult = true;

    inoutCommandList.EndEvent();
}

void PixelPickingPass::RequestPixelPick(unsigned aX, unsigned aY)
{
    myPendingPick = CommonUtilities::Vector2ui{ aX, aY };
}

bool PixelPickingPass::HasPendingPick() const
{
    return myPendingPick.has_value();
}

std::optional<unsigned> PixelPickingPass::TryGetPickResult(const RenderHardwareInterface& aRHI)
{
    if (!myHasAPickResult)
    {
        return std::nullopt;
    }

    myHasAPickResult = false;

    const unsigned id = aRHI.ReadObjectId(*myIDStagingBuffer);
    if (id == 0)
    {
        return std::nullopt;
    }

    return id - 1;
}