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
	CommonUtilities::Matrix4x4f UnrealToTGP(std::vector<float> someData);
	CommonUtilities::Matrix4x4f UnrealToTGP(CommonUtilities::Matrix4x4f someData);

private:
	std::vector<std::unique_ptr<Actor>> myActors;
};

