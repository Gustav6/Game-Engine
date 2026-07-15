#pragma once

#include <filesystem>	
#include <unordered_map>
#include <vector>
#include <memory>
#include <array>
#include <string>

#include "CommonUtilities/Matrix4x4.hpp"
#include "CommonUtilities/Frustum.hpp"

#include "RHI/RenderHardwareInterface.h"

#include "Materials/Material.h"

#include "Objects/Camera.h"

struct RHIShaderReflectionInfo;
class PipelineStateObject;
class Mesh;
struct Light;
class Shader;
class Texture;
class Sampler;

enum class ConstantBuffer : uint8_t
{
	FrameBuffer,
	ObjectBuffer,
	AnimationBuffer,
	LightBuffer,
	MaterialBuffer,
	MAX
};

struct DrawCall
{
	bool HasSkinning = false;
	const Mesh* Mesh = nullptr;
	const MaterialInterface* Materials[Material::MAX_MATERIAL_TEXTURE_COUNT] = {};
	CommonUtilities::Matrix4x4f Transform;
	const CommonUtilities::Matrix4x4f* JointTransforms = nullptr;
};

struct SceneCamera
{
	CommonUtilities::Matrix4x4f Transform;
	Camera Camera;
};

struct SceneLight
{
	const Light* LightData = nullptr;
	const Texture* ShadowMap = nullptr;
	CommonUtilities::Matrix4x4f Transform;
	CommonUtilities::Matrix4x4f Projection;
};

struct Scene
{
	std::vector<DrawCall> DrawCalls;
	std::vector<SceneLight> SceneLights;
	SceneCamera SceneCamera;
};

class GraphicsEngine
{
public:
	static constexpr unsigned AMOUNT_OF_CASCADES = 3;

	static GraphicsEngine& Get();

	bool Initialize(HWND aWindowHandle, const std::filesystem::path& aShaderRoot);
	void Render(GraphicsCommandList& inoutCommandList, const Scene& aScene);
	void Present() const;

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

	bool LoadTexture(const std::filesystem::path& aPath, Texture& outTexture) const;

private:
	GraphicsEngine();
	~GraphicsEngine();

	struct ShadowCascade
	{
		std::unique_ptr<Texture> ShadowMap = nullptr;
	};

	enum RenderDrawCallFlag_
	{
		RenderDrawCallFlag_None = 0,
		RenderDrawCallFlag_BindTextures = 1 << 0,
		RenderDrawCallFlag_BindMaterial = 1 << 1,
	};

	typedef int RenderDrawCallFlags;

	bool HasFlag(RenderDrawCallFlags aFlags, RenderDrawCallFlag_ aFlag);

	bool CreateConstantBufferInternal(ConstantBuffer aBufferID, std::string_view aName, size_t aBufferSize);
	bool UpdateAndSetConstantBufferInternal(GraphicsCommandList& inoutCommandList, ConstantBuffer aBufferID, const void* aData, size_t aDataSize, unsigned aSlot, PipelineStages aStages);

	bool PrepareMeshForRendering(const Mesh& aMesh) const;

	void CreateMaterialTextureSlots(const RHIShaderReflectionInfo& aShaderInfo, Material& inoutMaterial) const;

	void RenderShadowMaps(GraphicsCommandList& inoutCommandList, const Scene& aScene);
	
	void UploadSceneLightingData(GraphicsCommandList& inoutCommandList, const std::vector<SceneLight>& aLights);
	void RenderDrawCalls(GraphicsCommandList& inoutCommandList, const std::vector<DrawCall>& aDrawCalls, RenderDrawCallFlags someFlags);

	void RegisterMaterialShaders();
	void CreateShadowCascades(unsigned int aHighestCascadeSize);
	bool LoadDefaultAssets();
	bool CreateRenderResources();

	RenderHardwareInterface myRHI;

	std::filesystem::path myShaderRoot;

	std::unique_ptr<Texture> myBackBuffer;
	std::unique_ptr<Texture> myDepthBuffer;

	std::unordered_map<ConstantBuffer, Buffer> myConstantBuffers;

	std::unordered_map<MaterialDomain, std::filesystem::path> myMaterialDomainShaders;
	std::unordered_map<ShadingModel, std::filesystem::path> myMaterialShadingModelShaders;

	std::vector<std::unique_ptr<Sampler>> mySamplers;

	std::unique_ptr<Material> myDefaultMaterial;

	std::shared_ptr<Texture> myDefaultAlbedoTexture;
	std::shared_ptr<Texture> myDefaultNormalTexture;

	std::unique_ptr<PipelineStateObject> myShadowPSO;

	// First Index = Near Cascade ... last index = Far Cascade
	std::array<ShadowCascade, AMOUNT_OF_CASCADES> myCascades;
};