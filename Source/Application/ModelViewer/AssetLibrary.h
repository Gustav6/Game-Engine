#pragma once

#include <memory>
#include <unordered_map>
#include <filesystem>

struct Animation;
class Mesh;
class Texture;
class MaterialInterface;

class AssetLibrary
{
public:
	AssetLibrary();
	~AssetLibrary();

	/**
	 * Will initialize the AssetLibrary and load any default assets.
	 * @param aContentRoot The root directory for all content. All assets will be loaded relative to this path.
	 */
	void Initialize(const std::filesystem::path& aContentRoot);

	/**
	 * Will free any resources used by the FBX importer by uninitializing it.
	 * @return
	 */
	void UnInitialize();

	std::shared_ptr<Mesh> GetMesh(std::string_view aName);
	std::shared_ptr<Animation> GetAnimation(std::string_view aName);
	std::shared_ptr<Texture> GetTexture(std::string_view aName);
	std::shared_ptr<MaterialInterface> GetMaterial(std::string_view aName);

	bool LoadMesh(const std::filesystem::path& aPath);
	bool LoadAnimation(const std::filesystem::path& aPath);
	bool LoadTexture(const std::filesystem::path& aPath);
	bool LoadMaterial(const std::filesystem::path& aPath);

private:
	void InitUnitCube();
	void InitUnitPlane();

	// TODO: Not implemented
	void InitUnitAxes();

	std::filesystem::path myContentRoot;

	std::unordered_map<std::filesystem::path, std::shared_ptr<Mesh>> myMeshes;
	std::unordered_map<std::filesystem::path, std::shared_ptr<Animation>> myAnimations;
	std::unordered_map<std::filesystem::path, std::shared_ptr<Texture>> myTextures;
	std::unordered_map<std::filesystem::path, std::shared_ptr<MaterialInterface>> myMaterials;
};