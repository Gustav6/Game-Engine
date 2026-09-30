#pragma once

#include <filesystem>
#include <vector>
#include <memory>

#include <AssetPipeline/Assets/Base/Asset.h>

#include "GamePipeline/World/Actor.hpp"

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
	CommonUtilities::Matrix4x4f UnrealToDirectX(const std::vector<float>& someData);
	CommonUtilities::Matrix4x4f UnrealToDirectX(const CommonUtilities::Matrix4x4f& someData);

private:

	void PrintVector(float* aVector);
	void PrintRot(float* aVector, bool aUEStyle);
	void PrintTrx(float* aVector, bool aUEStyle);

	void Decompose(const CommonUtilities::Matrix4x4f& m, float* outTranslation, float* outRotation, float* outScale);

	void PrintMatrix(CommonUtilities::Matrix4x4f& aMatrix);


	std::vector<std::unique_ptr<Actor>> myActors;
};

