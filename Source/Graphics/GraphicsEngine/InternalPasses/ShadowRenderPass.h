#pragma once

#include <array>
#include <filesystem>

class GraphicsCommandList;
class Texture;
class PipelineStateObject;
class RenderHardwareInterface;

struct RenderContext;
struct RenderItem;

struct ShadowCascade
{
	std::unique_ptr<Texture> ShadowMap = nullptr;
	float NearPlane = 0.0f;
	float FarPlane = 0.0f;
};

class ShadowRenderPass
{
public:
	static constexpr unsigned int NUM_CASCADES = 4;

	bool Initialize(RenderHardwareInterface& aRHI);

    void Execute(
		GraphicsCommandList& inoutCommandList,
		const std::vector<RenderItem>& aItems,
		RenderContext& aContext
    );

	const std::array<ShadowCascade, NUM_CASCADES>& GetCascades();

private:
	/**
	 * Will create the shadow cascades with specified sizes.
	 * @param aCascadeSizes Sizes will apply in order from closest to furthest away.
	 */
	void BuildShadowCascades(
		RenderContext& aContext,
		const std::array<int, NUM_CASCADES>& aCascadeSizes, 
		float aCameraNear, 
		float aCameraFar
	);

	std::array<ShadowCascade, NUM_CASCADES> myCascades;
	std::unique_ptr<PipelineStateObject> myShadowPSO;
	std::unique_ptr<PipelineStateObject> myShadowCubePSO;

	float myCameraNearPlane = 0.0f;
	float myCameraFarPlane = 0.0f;
	float myShadowCascadePlaneChangeThreshold = 0.0f;
};