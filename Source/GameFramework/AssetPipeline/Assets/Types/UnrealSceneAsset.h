#pragma once

#include <filesystem>
#include <vector>
#include <memory>

#include <CommonUtilities/Math/Matrix4x4.hpp>

#include <AssetPipeline/Assets/Base/Asset.h>

class MaterialAsset;
class Actor;

enum class ComponentType
{
	SceneComponent = 0,
	StaticMeshComponent,
	SkeletalMeshComponent,
	PointLightComponent,
	SpotLightComponent,
	DirectionalLightComponent,
	BoxComponent,
	SphereComponent,
	CapsuleComponent,
	SpringArmComponent,
	COUNT
};

class UnrealSceneAsset : public Asset
{
	friend class AssetRegistry;

public:
	~UnrealSceneAsset() override;

	std::vector<std::unique_ptr<Actor>>& GetSceneActors();

protected:
	bool Load(const std::filesystem::path& aPath) override;

private:
	CommonUtilities::Matrix4x4f UnrealToDirectX(const std::vector<float>& someData);
	CommonUtilities::Matrix4x4f UnrealToDirectX(const CommonUtilities::Matrix4x4f& someData);
	void Decompose(const CommonUtilities::Matrix4x4f& m, float* outTranslation, float* outRotation, float* outScale);

	void PrintVector(float* aVector);
	void PrintRot(float* aVector, bool aUEStyle);
	void PrintTrx(float* aVector, bool aUEStyle);
	void PrintMatrix(CommonUtilities::Matrix4x4f& aMatrix);

	std::vector<std::unique_ptr<Actor>> myActors;
	std::vector<std::shared_ptr<MaterialAsset>> myMasterMaterials;

	float myLightMultiplier = 1000000.0f;
};

