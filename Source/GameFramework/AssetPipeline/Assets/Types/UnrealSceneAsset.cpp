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

			//actor->myName = jsonActor.GetString("Name");

			actor->GetTransform() = UnrealToDirectX(matrix);

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

					//AnimationComponent* animationComponent = actor->AddComponent<AnimationComponent>(meshComponent->GetSkeleton());

					//AnimationEntry breathingAnimation =
					//{
					//	.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Idle_Breathing"),
					//	.ShouldLoop = true
					//};
					//AnimationEntry waveAnimation =
					//{
					//	.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Idle_Wave"),
					//	.ShouldLoop = true
					//};
					//AnimationEntry runAnimation =
					//{
					//	.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Run"),
					//	.ShouldLoop = true
					//};
					//AnimationEntry walkAnimation =
					//{
					//	.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_C_TGA_Bro_Walk"),
					//	.ShouldLoop = true
					//};

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
		}
	}
	else
	{
		LOG(SceneLoaderLog, Warning, "No Actors found in scene [{}]. Scene might be empty", aPath.string());
	}

	return true;
}

CommonUtilities::Matrix4x4f UnrealSceneAsset::UnrealToDirectX(const std::vector<float>& someData)
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

CommonUtilities::Matrix4x4f UnrealSceneAsset::UnrealToDirectX(const CommonUtilities::Matrix4x4f& someData)
{
	CommonUtilities::Matrix4x4f matrix =
	{
		someData(2,2), someData(2,3), someData(2,1), 0.0f,
		someData(3,2), someData(3,3), someData(3,1), 0.0f,
		someData(1,2), someData(1,3), someData(1,1), 0.0f,
		someData(4,2), someData(4,3), someData(4,1), 1.0f
	};

	return matrix;
}

void UnrealSceneAsset::PrintVector(float* aVector)
{
	static char axis[] = { 'X', 'Y', 'Z' };

	for (int i = 0; i < 3; ++i)
	{
		std::cout << " " << axis[i] << ": " << aVector[i];
	}
	std::cout << "\n";
}

void UnrealSceneAsset::PrintRot(float* aVector, bool aUEStyle)
{
	static char axis[] = { 'P', 'Y', 'R' };
	static char ueAxis[] = { 'R', 'P', 'Y' };

	for (int i = 0; i < 3; ++i)
	{
		std::cout << " " << (aUEStyle ? ueAxis[i] : axis[i]) << ": " << aVector[i];
	}
	std::cout << "\n";
}

void UnrealSceneAsset::PrintTrx(float* aVector, bool aUEStyle)
{
	static char axis[] = { 'R', 'U', 'F' };
	static char ueAxis[] = { 'F', 'R', 'U' };

	for (int i = 0; i < 3; ++i)
	{
		std::cout << " " << (aUEStyle ? ueAxis[i] : axis[i]) << ": " << aVector[i];
	}
	std::cout << "\n";
}

void UnrealSceneAsset::Decompose(const CommonUtilities::Matrix4x4f& m, float* outTranslation, float* outRotation, float* outScale)
{
	outTranslation[0] = m(4, 1);
	outTranslation[1] = m(4, 2);
	outTranslation[2] = m(4, 3);

	outScale[0] = std::sqrt(m(1, 1) * m(1, 1) + m(1, 2) * m(1, 2) + m(1, 3) * m(1, 3));
	outScale[1] = std::sqrt(m(2, 1) * m(2, 1) + m(2, 2) * m(2, 2) + m(2, 3) * m(2, 3));
	outScale[2] = std::sqrt(m(3, 1) * m(3, 1) + m(3, 2) * m(3, 2) + m(3, 3) * m(3, 3));

	if (outScale[0] == 0.0f ||
		outScale[1] == 0.0f ||
		outScale[2] == 0.0f)
	{
		outRotation[0] = 0.0f;
		outRotation[1] = 0.0f;
		outRotation[2] = 0.0f;
		return;
	}

	const float r00 = m(1, 1) / outScale[0];
	const float r01 = m(1, 2) / outScale[0];
	//const float r02 = m(1, 3) / outScale[0];

	const float r10 = m(2, 1) / outScale[1];
	const float r11 = m(2, 2) / outScale[1];
	//const float r12 = m(2, 3) / outScale[1];

	const float r20 = m(3, 1) / outScale[2];
	const float r21 = m(3, 2) / outScale[2];
	const float r22 = m(3, 3) / outScale[2];

	float pitch;
	float yaw;
	float roll;

	yaw = std::asin(-r20);

	if (std::abs(r20) < 0.9999f)
	{
		pitch = std::atan2(r21, r22);
		roll = std::atan2(r10, r00);
	}
	else
	{
		pitch = 0.0f;
		roll = std::atan2(-r01, r11);
	}

	constexpr float RadToDeg = 180.0f / 3.14159265359f;

	outRotation[0] = pitch * RadToDeg;
	outRotation[1] = yaw * RadToDeg;
	outRotation[2] = roll * RadToDeg;
}

void UnrealSceneAsset::PrintMatrix(CommonUtilities::Matrix4x4f& aMatrix)
{
	for (int i = 0; i < 16; ++i)
	{
		int r = i / 4;
		int c = i % 4;
		std::cout << aMatrix(r + 1, c + 1) << " ";
		if (c == 3)
			std::cout << "\n";
	}
}


