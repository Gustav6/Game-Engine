#pragma once

/*
* GraphicsEngine handles rendering of a scene.
* Handles culling using whatever cameras it's told to use.
* Renders onto a user-provided render target and depth.
*/

#include <filesystem>	
#include <unordered_map>
#include <vector>
#include <memory>

#include "CommonUtilities/Math/Vector2.hpp"

#include "Materials/Material.h"

#include "Core/SceneCamera.h"
#include "Core/DebugRenderMode.h"

#include "Objects/GPU/Buffer.h"

class RenderHardwareInterface;
class GraphicsCommandList;
class Texture;
class Sampler;
class Font;
class Text;

class BuiltinAssets;
class MaterialShaderRegistry;
class ConstantBufferManager;

class ForwardRenderPass;
class DeferredRenderPass;
class ScreenSpaceRenderPass;
class ShadowRenderPass;

class GraphicsEngine
{
public:
	static GraphicsEngine& Get();

	bool Initialize(HWND aWindowHandle, const std::filesystem::path& aShaderRoot);

	void BeginFrame();
	void Render(GraphicsCommandList& inoutCommandList, const SceneCamera& aCamera);
	void Present() const;

	void CycleToNextRenderPass(int aDirection);

	CommonUtilities::Vector2ui GetClientSize() const;

	bool CreateCommandList(std::string_view aName, GraphicsCommandList& outCommandList) const;
	void ExecuteCommandList(const GraphicsCommandList& aCommandList) const;

	bool CreateMaterial(const MaterialDescription& aDescription, Material& outMaterial) const;
	bool CreateShadowMap(std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outShadowMap, bool aCube) const;
	bool CreateTexture(std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture) const;
	bool CreateTexture(const std::filesystem::path& aPath, Texture& outTexture) const;
	bool CreateFontAsset(const std::filesystem::path& aFontPath, const std::filesystem::path& aFontAtlasPath, Font& outFont) const;

	/**
	 * Will set the texture for the environment, which is used in PBL.
	 * @param aEnvironmentTexture A texture which is in Cubemap format.
	 */
	void SetEnvironmentTexture(std::unique_ptr<Texture> aEnvironmentTexture);

private:
	GraphicsEngine();
	~GraphicsEngine();

	bool InitializeRenderResources();

	std::filesystem::path myShaderRoot;
	std::filesystem::path myContentRoot;

	std::unique_ptr<RenderHardwareInterface> myRHI;

	std::unique_ptr<BuiltinAssets> myBuiltinAssets;
	std::unique_ptr<MaterialShaderRegistry> myMaterialShaderRegistry;
	std::unique_ptr<ConstantBufferManager> myConstantBufferManager;

	std::unique_ptr<ForwardRenderPass> myForwardRenderPass;
	std::unique_ptr<DeferredRenderPass> myDeferredRenderPass;
	std::unique_ptr<ScreenSpaceRenderPass> myScreenSpaceRenderPass;
	std::unique_ptr<ShadowRenderPass> myShadowRenderPass;

	std::unique_ptr<Texture> myBackBuffer;
	std::unique_ptr<Texture> myDepthBuffer;

	std::vector<std::unique_ptr<Sampler>> mySamplers;

	std::unique_ptr<Texture> myEnvironmentCubemap;
	std::unique_ptr<Texture> myLUTTexture;

	DebugRenderMode myDebugRenderMode = DebugRenderMode::sRGB;
};