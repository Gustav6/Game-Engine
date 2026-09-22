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

#include "CommonUtilities/Math/Matrix4x4.hpp"
#include "Objects/Buffer.h"

#include "Materials/Material.h"

class RenderHardwareInterface;
struct RHIShaderReflectionInfo;
class PipelineStateObject;
class Shader;
class Texture;
class Sampler;
struct InternalMeshRenderItem;
class GBuffer;
class GraphicsCommandList;
class Font;
class Text;
class Mesh;
class Camera;

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

struct SceneCamera
{
	CommonUtilities::Matrix4x4f Transform;
	const Camera* RenderCamera = nullptr;
};

class GraphicsEngine
{
public:
	static constexpr unsigned NUM_CASCADES = 4;
	
	static GraphicsEngine& Get();

	bool Initialize(HWND aWindowHandle, const std::filesystem::path& aShaderRoot);
	void BeginFrame();
	void Render(GraphicsCommandList& inoutCommandList, const SceneCamera& aCamera);
	void Present() const;

	void CycleToNextRenderPass(int aDirection);

	template<class T>
	bool CreateConstantBuffer(ConstantBuffer aBufferID, std::string_view aName)
	{
		return CreateConstantBufferInternal(aBufferID, aName, sizeof(T));
	}

	bool CreateConstantBuffer(ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize);

	template<class T>
	bool UpdateAndSetConstantBuffer(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const T& aData, unsigned aSlot, PipelineStages aStages)
	{
		return UpdateAndSetConstantBufferInternal(inoutCommandList, aBufferID, &aData, sizeof(T), aSlot, aStages);
	}

	CommonUtilities::Vector2ui GetClientSize() const;

	bool CreateCommandList(std::string_view aName, GraphicsCommandList& outCommandList) const;
	void ExecuteCommandList(const GraphicsCommandList& aCommandList) const;

	bool CreateMaterial(const MaterialDescription& aDescription, Material& outMaterial) const;
	bool CreateShadowMap(std::string_view aName, unsigned int aWidth, unsigned int aHeight, Texture& outShadowMap, bool aCube) const;
	bool CreateTexture(std::string_view aName, unsigned aWidth, unsigned aHeight, unsigned aNumMips, ResourceFormat aFormat, ResourceBindFlags aBindFlags, Texture& outTexture) const;
	bool CreateFontAsset(const std::filesystem::path& aFontPath, const std::filesystem::path& aFontAtlasPath, Font& outFont) const;

	bool LoadTexture(const std::filesystem::path& aPath, Texture& outTexture) const;

	/**
	 * Will set the texture for the environment, which is used in PBL.
	 * @param aEnvironmentTexture A texture which is in Cubemap format.
	 */
	void SetEnvironmentTexture(std::unique_ptr<Texture> aEnvironmentTexture);

private:
	GraphicsEngine();
	~GraphicsEngine();

	struct ShadowCascade
	{
		std::unique_ptr<Texture> ShadowMap = nullptr;
		float NearPlane = 0.0f;
		float FarPlane = 0.0f;
	};

	enum DrawCallFlag_
	{
		DrawCallFlag_None = 0,
		DrawCallFlag_BindTextures = 1 << 0,
		DrawCallFlag_BindMaterial = 1 << 1
	};

	typedef int DrawCallFlags;

	enum class RenderPass
	{
		sRGB,
		Roughness,
		Metalness,
		AmbientOcclusionTexture,
		AmbientOcclusionScreenspace,
		NormalsTangentSpace,
		NormalsWorldSpace,
		Shadows,
		MAX
	};

	bool HasDrawCallFlag(DrawCallFlags aFlags, DrawCallFlag_ aFlag);

	bool CreateConstantBufferInternal(ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize);
	bool UpdateAndSetConstantBufferInternal(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const void* aData, size_t aDataSize, unsigned aSlot, PipelineStages aStages);

	bool PrepareMeshForRendering(const Mesh& aMesh) const;

	void CreateMaterialTextureSlots(const RHIShaderReflectionInfo& aShaderInfo, Material& inoutMaterial) const;

	void RenderShadowMaps(GraphicsCommandList& inoutCommandList, const SceneCamera& aCamera, std::vector<InternalMeshRenderItem>& aShadowItems);
	
	void UploadSceneLightingData(GraphicsCommandList& inoutCommandList);
	void RenderDrawCalls(GraphicsCommandList& inoutCommandList, const std::vector<InternalMeshRenderItem>& aRenderItems, DrawCallFlags someFlags);

	void RegisterMaterialShaders();

	/**
	 * Will create the shadow cascades with specified sizes.
	 * @param aCascadeSizes Sizes will apply in order from closest to furthest away.
	 */
	void BuildShadowCascades(const std::array<int, NUM_CASCADES>& aCascadeSizes, float aCameraNear, float aCameraFar);
	bool InitializeBuiltinAssets();
	bool InitializeRenderResources();

	std::unique_ptr<RenderHardwareInterface> myRHI;

	std::filesystem::path myShaderRoot;
	std::filesystem::path myContentRoot;

	std::unique_ptr<Texture> myBackBuffer;
	std::unique_ptr<Texture> myDepthBuffer;

	std::unordered_map<ConstantBuffer, Buffer> myConstantBuffers;

	std::unordered_map<MaterialDomain, std::filesystem::path> myMaterialDomainShaders;
	std::unordered_map<ShadingModel, std::filesystem::path> myMaterialShadingModelShaders;

	std::vector<std::unique_ptr<Sampler>> mySamplers;

	std::unique_ptr<Material> myDefaultMaterial;
	std::shared_ptr<Material> myDefaultFontMaterial;

	std::shared_ptr<Texture> myDefaultAlbedoTexture;
	std::shared_ptr<Texture> myDefaultNormalTexture;

	std::unique_ptr<Texture> myEnvironmentCubemap;
	std::unique_ptr<Texture> myLUTTexture;

	std::unique_ptr<GBuffer> myGBuffer;

	std::unique_ptr<PipelineStateObject> myShadowPSO;
	std::unique_ptr<PipelineStateObject> myGBufferPSO;
	std::unique_ptr<PipelineStateObject> myDeferredAmbientPSO;
	std::unique_ptr<PipelineStateObject> myDeferredDirPSO;
	std::unique_ptr<PipelineStateObject> myDeferredSpotPSO;
	std::unique_ptr<PipelineStateObject> myDeferredPointPSO;

	// First Index = Near Cascade ... last index = Far Cascade
	std::array<ShadowCascade, NUM_CASCADES> myCascades;

	RenderPass myCurrentRenderPass = RenderPass::sRGB;
};