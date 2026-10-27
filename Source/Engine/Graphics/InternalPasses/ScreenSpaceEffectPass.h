#pragma once

#include <memory>
#include <array>

#include "CommonUtilities/Math/Vector2.hpp"

class GraphicsCommandList;
class RenderHardwareInterface;
class PipelineStateObject;
class Texture;

struct RenderContext;
struct FrameTargets;

class ScreenSpaceEffectPass
{
public:
	bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(
        GraphicsCommandList& inoutCommandList,
        FrameTargets& aFrameTargets,
        RenderContext& aContext,
        Texture* aHDRTexture
    );

    void SetBloomStatus(bool aEnabled);

private:
	struct BloomMip
	{
	    CommonUtilities::Vector2ui Size;
        std::unique_ptr<Texture> Texture;
	};

    void BloomPass(
        GraphicsCommandList& inoutCommandList,
        RenderContext& aContext,
        Texture* aHDRTexture);

    void TonemapPass(
        GraphicsCommandList& inoutCommandList,
        RenderContext& aContext,
        Texture* aHDRTexture);

    std::unique_ptr<PipelineStateObject> myTonemapPSO;
    std::unique_ptr<PipelineStateObject> myLuminancePSO;
    std::unique_ptr<PipelineStateObject> myBloomPSO;

    std::unique_ptr<Texture> mySceneLuminance;

    std::array<BloomMip, 6> myBloomMips;

    bool myBloomEnabled;
};
