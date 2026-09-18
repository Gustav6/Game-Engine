#include "GameFramework.pch.h"
#include "UnrealSceneAsset.h"

#include <memory.h>

#include "CommonUtilities/Json/JsonWrapper.h"

#include "AssetPipeline/AssetRegistry/AssetRegistry.h"
#include "AssetPipeline/Assets/Types/MeshAsset.h"
#include "AssetPipeline/Assets/Types/AnimationAsset.h"
#include "AssetPipeline/Assets/Types/MaterialAsset.h"
#include "AssetPipeline/Assets/Types/TextureAsset.h"

#include "GamePipeline/Components/Rendering/MeshComponent.h"
#include "GamePipeline/Components/Rendering/AnimationComponent.h"
#include "GamePipeline/Components/Light/DirectionlLightComponent.h"
#include "GamePipeline/Components/Light/PointLightComponent.h"
#include "GamePipeline/Components/Light/SpotLightComponent.h"
#include "GamePipeline/World/GameWorld.h"
#include "GamePipeline/World/Actor.hpp"

#include "Logger/Logger.h"

#include <tracy/core/Tracy.hpp>

DECLARE_LOG_CATEGORY_WITH_NAME(SceneLoaderLog, SCENELOADER, Verbose);

DEFINE_LOG_CATEGORY(SceneLoaderLog);

UnrealSceneAsset::~UnrealSceneAsset() = default;

std::vector<std::unique_ptr<Actor>>& UnrealSceneAsset::GetSceneActors()
{
	return myActors;
}

bool UnrealSceneAsset::Load(const std::filesystem::path& aPath)
{
	ZoneScopedN("Load Unreal Scene");
	myActors.clear();

	JsonData unrealSceneData = Json::Read(aPath);

	if (!unrealSceneData.IsValid())
	{
		LOG(SceneLoaderLog, Error, "Failed to load Scene from JSON [{}]", aPath.string());
		return false;
	}

	if (unrealSceneData.HasKey("Actors"))
	{
		std::vector<JsonData> JsonActors = unrealSceneData.GetObjectValueArray("Actors");
		std::vector<Actor> actors;

		for (auto& jsonActor : JsonActors)
		{
			std::unique_ptr<Actor> actor = std::make_unique<Actor>();

			std::vector<float> matrix = jsonActor.GetFloatArray("Transform");

			actor->myTransform = UnrealToTGP(matrix);

			std::vector<JsonData> jsonComponents = jsonActor.GetObjectValueArray("Components");

			for (auto& jsonComponent : jsonComponents)
			{
				ComponentType componentType = static_cast<ComponentType>(jsonComponent.GetInt("TypeID"));

				switch (componentType)
				{
				case ComponentType::SceneComponent:
					break;
				case ComponentType::StaticMeshComponent:
				{
					std::string fullMeshPath = jsonComponent.GetString("ContentPath");

					size_t pos = fullMeshPath.find(".");
					std::string meshPath = fullMeshPath.substr(pos + 1);

					MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();
					meshComponent->SetMesh(AssetRegistry::Get().GetAsset<MeshAsset>(meshPath));

					std::vector<JsonData> materials = jsonComponent.GetObjectValueArray("Materials");
					for (unsigned int i = 0; i < materials.size(); i++)
					{
						std::string materialName = materials[i].GetString("Name");

						auto material = AssetRegistry::Get().GetAsset<MaterialAsset>(materialName);

						if (material != nullptr)
						{
							meshComponent->SetMaterial(i, material);
						}
						else
						{
							meshComponent->SetMaterial(i, AssetRegistry::Get().GetAsset<MaterialAsset>("ErrorMaterial"));
						}

					}
				}
				break;
				case ComponentType::SkeletalMeshComponent:
					break;
				case ComponentType::PointLightComponent:
				{
					PointLightComponent* spotLight = actor->AddComponent<PointLightComponent>();
					std::vector<float> jsonColor = jsonComponent.GetFloatArray("Color");
					CommonUtilities::Vector3f color = { jsonColor[0],jsonColor[1] ,jsonColor[2] };

					spotLight->SetColor(color);
					spotLight->SetIntensity(jsonComponent.GetFloat("Intensity"));
					spotLight->SetRadius(jsonComponent.GetFloat("AttenuationRadius"));
				}
					break;
				case ComponentType::SpotLightComponent:

				{
					SpotLightComponent* spotLight = actor->AddComponent<SpotLightComponent>();
					std::vector<float> jsonColor = jsonComponent.GetFloatArray("Color");
					CommonUtilities::Vector3f color = { jsonColor[0],jsonColor[1] ,jsonColor[2] };

					float outerConeAngle = jsonComponent.GetFloat("OuterConeAngle");
					float outerConeRadius = outerConeAngle * CommonUtilities::Math::DEG_TO_RAD<float>;

					spotLight->SetColor(color);
					spotLight->SetIntensity(jsonComponent.GetFloat("Intensity"));
					spotLight->SetInnerCone(jsonComponent.GetFloat("InnerConeAngle"));
					spotLight->SetOuterCone(outerConeRadius);
					spotLight->SetRadius(jsonComponent.GetFloat("AttenuationRadius"));
				}

					break;
				case ComponentType::DirectionalLightComponent:
				{
					DirectionalLightComponent* directionalLight = actor->AddComponent<DirectionalLightComponent>();

					std::vector<float> jsonColor = jsonComponent.GetFloatArray("Color");
					CommonUtilities::Vector3f color = { jsonColor[0],jsonColor[1] ,jsonColor[2] };

					directionalLight->SetColor(color);
					directionalLight->SetIntensity(jsonComponent.GetFloat("Intensity"));

				}
					break;
				case ComponentType::BoxComponent:
					break;
				case ComponentType::SphereComponent:
					break;
				case ComponentType::CapsuleComponent:
					break;
				case ComponentType::SpringArmComponent:
					break;
				case ComponentType::COUNT:
					break;
				default:
					break;
				}
			}

			GameWorld::Get().AddActor(std::move(actor));

			//myActors.push_back(std::move(actor));
		}
	}
	else
	{
		LOG(SceneLoaderLog, Warning, "No Actors found in scene [{}]. Scene might be empty", aPath.string());
	}

	return true;
}

CommonUtilities::Matrix4x4f UnrealSceneAsset::UnrealToTGP(std::vector<float> someData)
{
	CommonUtilities::Matrix4x4f matrix =
	{
		someData[5], someData[6], someData[4], 0.0f,
		someData[9], someData[10],someData[8], 0.0f,
		someData[1], someData[2], someData[0], 0.0f,
		someData[13],someData[14],someData[12],1.0f
	};

	return matrix;
}
