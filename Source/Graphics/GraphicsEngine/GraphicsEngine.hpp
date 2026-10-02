#pragma once

/*
* GraphicsEngine handles rendering of a scene.
* Handles culling using whatever cameras it's told to use.
* Renders onto a user-provided render target and depth.
*/

#include <filesystem>	
#include <vector>
#include <memory>

#include "CommonUtilities/Math/Vector2.hpp"

class RenderHardwareInterface;
class GraphicsCommandList;
class Texture;
class Sampler;
class Font;
class Material;

class BuiltinAssets;
class MaterialShaderRegistry;
class ConstantBufferManager;

class ForwardRenderPass;
class DeferredRenderPass;
class ScreenSpaceRenderPass;
class ShadowRenderPass;
class ScreenSpaceEffectPass;

struct RenderItem;
struct SceneCamera;
struct MaterialDescription;

enum class ResourceFormat;
enum class DebugRenderMode;
enum class ToneMappingOperator;

typedef int ResourceBindFlags;

class GraphicsEngine
{
public:
	static GraphicsEngine& Get();

	bool Initialize(HWND aWindowHandle, const std::filesystem::path& aShaderRoot);

	void BeginFrame();
	void Render(GraphicsCommandList& inoutCommandList, const SceneCamera& aCamera);
	void Present() const;

	void ToggleBloom() const;

	void CycleDebugRenderMode(int aDirection);
	void CycleToNextTonemap(int aDirection);

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
	void SetEnvironmentTexture(std::shared_ptr<Texture> aEnvironmentTexture);

private:
	GraphicsEngine();
	~GraphicsEngine();

	bool InitializeRenderResources();

	std::filesystem::path myShaderRoot;
	std::filesystem::path myContentRoot;

	DebugRenderMode myDebugRenderMode;
	ToneMappingOperator myToneMappingOperator;

	std::unique_ptr<RenderHardwareInterface> myRHI;

	std::unique_ptr<BuiltinAssets> myBuiltinAssets;
	std::unique_ptr<MaterialShaderRegistry> myMaterialShaderRegistry;
	std::unique_ptr<ConstantBufferManager> myConstantBufferManager;

	std::unique_ptr<ForwardRenderPass> myForwardRenderPass;
	std::unique_ptr<DeferredRenderPass> myDeferredRenderPass;
	std::unique_ptr<ScreenSpaceRenderPass> myScreenSpaceRenderPass;
	std::unique_ptr<ShadowRenderPass> myShadowRenderPass;
	std::unique_ptr<ScreenSpaceEffectPass> myScreenSpaceEffectPass;

	std::unique_ptr<Texture> myBackBuffer;
	std::unique_ptr<Texture> myHDRBuffer;
	std::unique_ptr<Texture> myDepthBuffer;

	std::vector<Sampler*> mySamplerPointers;
	std::vector<std::unique_ptr<Sampler>> mySamplers;

	std::shared_ptr<Texture> myEnvironmentCubemap;
	std::unique_ptr<Texture> myLUTTexture;

	std::vector<RenderItem> myOpaqueItems;
	std::vector<RenderItem> myAlphaItems;
	std::vector<RenderItem> myShadowItems;
};