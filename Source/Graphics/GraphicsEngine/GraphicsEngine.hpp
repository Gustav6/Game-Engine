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
#include <array>

#include "RenderPasses/Shared/SceneCamera.h"
#include "DebugRenderMode.h"

#include "Objects/Buffer.h"

#include "Materials/Material.h"

class RenderHardwareInterface;
struct RHIShaderReflectionInfo;
class PipelineStateObject;
class Shader;
class Texture;
class Sampler;
class GBuffer;
class GraphicsCommandList;
class Font;
class Text;
class Mesh;
class BuiltinAssets;

class AlphaRenderPass;
class OpaqueRenderPass;
class ScreenSpaceRenderPass;
class ShadowRenderPass;

enum class ConstantBuffer : uint8_t
{
	FrameBuffer,
	ObjectBuffer,
	AnimationBuffer,
	LightBuffer,
	MaterialBuffer,
	CascadeBuffer,
	MAX
};

class GraphicsEngine
{
public:
	static GraphicsEngine& Get();

	bool Initialize(HWND aWindowHandle, const std::filesystem::path& aShaderRoot);

	void BeginFrame();
	void Render(GraphicsCommandList& inoutCommandList, const SceneCamera& aCamera);
	void Present() const;

	void CycleToNextRenderPass(int aDirection);

	bool CreateConstantBuffer(ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize);

	template<class T>
	bool CreateConstantBuffer(ConstantBuffer aBufferID, std::string_view aName)
	{
		return CreateConstantBuffer(aBufferID, aName, sizeof(T));
	}

	bool UpdateAndSetConstantBuffer(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const void* aData, size_t aDataSize, unsigned aSlot, PipelineStages aStages);

	template<class T>
	bool UpdateAndSetConstantBuffer(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const T& aData, unsigned aSlot, PipelineStages aStages)
	{
		return UpdateAndSetConstantBuffer(inoutCommandList, aBufferID, &aData, sizeof(T), aSlot, aStages);
	}

	CommonUtilities::Vector2ui GetClientSize() const;

	bool CreateCommandList(std::string_view aName, GraphicsCommandList& outCommandList) const;
	void ExecuteCommandList(const GraphicsCommandList& aCommandList) const;

	bool CreateMaterial(const MaterialDescription& aDescription, Material& outMaterial) const;
	bool CreateShadowMap(std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outShadowMap, bool aCube) const;
	bool CreateTexture(std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture) const;
	bool CreateFontAsset(const std::filesystem::path& aFontPath, const std::filesystem::path& aFontAtlasPath, Font& outFont) const;
	bool CreateTexture(const std::filesystem::path& aPath, Texture& outTexture) const;

	/**
	 * Will set the texture for the environment, which is used in PBL.
	 * @param aEnvironmentTexture A texture which is in Cubemap format.
	 */
	void SetEnvironmentTexture(std::unique_ptr<Texture> aEnvironmentTexture);

private:
	GraphicsEngine();
	~GraphicsEngine();

	void CreateMaterialTextureSlots(const RHIShaderReflectionInfo& aShaderInfo, Material& inoutMaterial) const;

	bool InitializeRenderResources();

	std::unique_ptr<RenderHardwareInterface> myRHI;

	std::unique_ptr<BuiltinAssets> myBuiltinAssets;

	std::unique_ptr<AlphaRenderPass> myAlphaRenderPass;
	std::unique_ptr<OpaqueRenderPass> myOpaqueRenderPass;
	std::unique_ptr<ScreenSpaceRenderPass> myScreenSpaceRenderPass;
	std::unique_ptr<ShadowRenderPass> myShadowRenderPass;

	std::filesystem::path myShaderRoot;
	std::filesystem::path myContentRoot;

	std::unique_ptr<Texture> myBackBuffer;
	std::unique_ptr<Texture> myDepthBuffer;

	std::unordered_map<ConstantBuffer, Buffer> myConstantBuffers;

	std::unordered_map<MaterialDomain, std::filesystem::path> myMaterialDomainShaders;
	std::unordered_map<ShadingModel, std::filesystem::path> myMaterialShadingModelShaders;

	std::vector<std::unique_ptr<Sampler>> mySamplers;

	std::unique_ptr<Texture> myEnvironmentCubemap;
	std::unique_ptr<Texture> myLUTTexture;

	DebugRenderMode myDebugRenderMode = DebugRenderMode::sRGB;
};