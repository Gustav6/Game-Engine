#include "GameFramework.pch.h"
#include "UnrealSceneAsset.h"

#include <memory.h>

#include "Json/JsonWrapper.h"

#include "AssetPipeline/AssetRegistry/AssetRegistry.h"
#include "AssetPipeline/Assets/Types/MeshAsset.h"
#include "AssetPipeline/Assets/Types/AnimationAsset.h"
#include "AssetPipeline/Assets/Types/MaterialAsset.h"
#include "AssetPipeline/Assets/Types/TextureAsset.h"

#include "GraphicsEngine/Objects/GPU/Texture.h"

#include "GamePipeline/Components/Rendering/MeshComponent.h"
#include "GamePipeline/Components/Rendering/AnimationComponent.h"
#include "GamePipeline/Components/Light/DirectionlLightComponent.h"
#include "GamePipeline/Components/Light/PointLightComponent.h"
#include "GamePipeline/Components/Light/SpotLightComponent.h"
#include "GamePipeline/World/GameWorld.h"
#include "GamePipeline/World/Actor.hpp"

#include "Logger/Logger.h"

#include <tracy/core/Tracy.hpp>

UnrealSceneAsset::~UnrealSceneAsset() = default;

std::vector<std::unique_ptr<Actor>>& UnrealSceneAsset::GetSceneActors()
{
	return myActors;
}

bool UnrealSceneAsset::Load(const std::filesystem::path& aPath)
{
	ZoneScopedN("Load Unreal Scene")

	Json::Data unrealSceneData = Json::Read(aPath);

	if (!unrealSceneData.IsValid())
	{
		SPDLOG_ERROR("Failed to load Scene from JSON [{}]", aPath.string());
		return false;
	}

	myMasterMaterials.clear();

	if (unrealSceneData.HasKey("MasterMaterials"))
	{
		std::vector<Json::Data> jsonMaterials = unrealSceneData.GetDataObjectArray("MasterMaterials");

		for (const auto& masterMaterial : jsonMaterials)
		{
			std::string name = masterMaterial.GetString("Name");

			myMasterMaterials.emplace_back(AssetRegistry::Get().ConstructAsset<MaterialAsset>(name, masterMaterial));
		}
	}
	else
	{
		SPDLOG_WARN("No Master Materials found in scene [{}]. Scene might be empty", aPath.string());
	}


	myActors.clear();

	if (unrealSceneData.HasKey("Actors"))
	{
		std::vector<Json::Data> jsonActors = unrealSceneData.GetDataObjectArray("Actors");

		for (const auto& jsonActor : jsonActors)
		{
			std::unique_ptr<Actor> actor = std::make_unique<Actor>();

			std::string actorName = jsonActor.GetString("Name");

			std::vector<float> matrix = jsonActor.GetFloatArray("Transform");

			actor->GetTransform() = UnrealToDirectX(matrix);

			std::vector<Json::Data> jsonComponents = jsonActor.GetDataObjectArray("Components");

			for (auto& jsonComponent : jsonComponents)
			{
				ComponentType componentType = static_cast<ComponentType>(jsonComponent.GetInt("TypeID"));

				switch (componentType)
				{
					case ComponentType::SceneComponent:
					{
						break;
					}
					case ComponentType::StaticMeshComponent:
					{
						std::shared_ptr<MeshAsset> mesh = AssetRegistry::Get().GetAsset<MeshAsset>(jsonComponent.GetString("Mesh"));

						if (mesh == nullptr)
						{
							break;
						}

						MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();
						meshComponent->SetMesh(mesh);

						std::vector<Json::Data> materials = jsonComponent.GetDataObjectArray("Materials");

						if (materials.empty())
						{
							SPDLOG_ERROR("Mesh has no material set [{}]!", actorName);
						}
						else
						{
							for (unsigned int i = 0; i < materials.size(); i++)
							{
								std::string materialName;

								if (materials[i].HasKey("Name"))
								{
									materialName = materials[i].GetString("Name");
								}
								else
								{
									SPDLOG_ERROR("Material had no name for actor [{}]!", actorName);
									materialName = "NO NAME GIVEN";
								}

								meshComponent->SetMaterial(i, AssetRegistry::Get().ConstructAsset<MaterialAsset>(materialName, materials[i]));
							}
						}

						break;
					}
					case ComponentType::SkeletalMeshComponent:
					{
						std::shared_ptr<MeshAsset> mesh = AssetRegistry::Get().GetAsset<MeshAsset>(jsonComponent.GetString("Mesh"));

						if (mesh == nullptr)
						{
							break;
						}

						MeshComponent* meshComponent = actor->AddComponent<MeshComponent>();
						meshComponent->SetMesh(mesh);

						std::vector<Json::Data> materials = jsonComponent.GetDataObjectArray("Materials");

						if (materials.empty())
						{
							SPDLOG_ERROR("Mesh has no material set [{}]!", actorName);
						}
						else
						{
							for (unsigned int i = 0; i < materials.size(); i++)
							{
								std::string materialName;

								if (materials[i].HasKey("Name"))
								{
									materialName = materials[i].GetString("Name");
								}
								else
								{
									SPDLOG_ERROR("Material had no name for actor [{}]!", actorName);
									materialName = "NO NAME GIVEN";
								}

								meshComponent->SetMaterial(i, AssetRegistry::Get().ConstructAsset<MaterialAsset>(materialName, materials[i]));
							}
						}

						AnimationComponent* animationComponent = actor->AddComponent<AnimationComponent>(meshComponent->GetSkeleton());

						AnimationEntry idleAnimation =
						{
							.AnimationAsset = AssetRegistry::Get().GetAsset<AnimationAsset>("A_swampGrass"),
							.ShouldLoop = true
						};

						animationComponent->AddAnimation("Idle", idleAnimation);

						break;
					}
					case ComponentType::PointLightComponent:
					{
						if (!jsonComponent.HasKey("Color"))
						{
							SPDLOG_ERROR("Point Light has no Color [{}]!", actorName);
							break;
						}
						if (!jsonComponent.HasKey("Intensity"))
						{
							SPDLOG_ERROR("Point Light has no Intensity [{}]!", actorName);
							break;
						}
						if (!jsonComponent.HasKey("AttenuationRadius"))
						{
							SPDLOG_ERROR("Point Light has no Attenuation Radius [{}]!", actorName);
							break;
						}

						PointLightComponent* pointLight = actor->AddComponent<PointLightComponent>();
						std::vector<float> jsonColor = jsonComponent.GetFloatArray("Color");
						CommonUtilities::Vector3f color = { jsonColor[0],jsonColor[1] ,jsonColor[2] };

						pointLight->SetColor(color);
						pointLight->SetPosition(actor->GetTransform().GetPosition());
						pointLight->SetIntensity(jsonComponent.GetFloat("Intensity") * myLightMultiplier);
						pointLight->SetRadius(jsonComponent.GetFloat("AttenuationRadius"));
						pointLight->SetShadowMapSize({ 512, 512 });

						break;
					}
					case ComponentType::SpotLightComponent:
					{
						if (!jsonComponent.HasKey("Color"))
						{
							SPDLOG_ERROR("Spot Light has no Color [{}]!", actorName);
							break;
						}
						if (!jsonComponent.HasKey("Intensity"))
						{
							SPDLOG_ERROR("Spot Light has no Intensity [{}]!", actorName);
							break;
						}
						if (!jsonComponent.HasKey("AttenuationRadius"))
						{
							SPDLOG_ERROR("Spot Light has no Attenuation Radius [{}]!", actorName);
							break;
						}
						if (!jsonComponent.HasKey("InnerConeAngle"))
						{
							SPDLOG_ERROR("Spot Light has no Inner Cone Angle [{}]!", actorName);
							break;
						}
						if (!jsonComponent.HasKey("OuterConeAngle"))
						{
							SPDLOG_ERROR("Spot Light has no Outer Cone Angle [{}]!", actorName);
							break;
						}

						SpotLightComponent* spotLight = actor->AddComponent<SpotLightComponent>();
						std::vector<float> jsonColor = jsonComponent.GetFloatArray("Color");
						CommonUtilities::Vector3f color = { jsonColor[0],jsonColor[1] ,jsonColor[2] };

						spotLight->SetColor(color);
						spotLight->SetIntensity(jsonComponent.GetFloat("Intensity") * myLightMultiplier);
						spotLight->SetInnerCone(jsonComponent.GetFloat("InnerConeAngle") * CommonUtilities::Math::DEG_TO_RAD<float>);
						spotLight->SetOuterCone(jsonComponent.GetFloat("OuterConeAngle") * CommonUtilities::Math::DEG_TO_RAD<float>);
						spotLight->SetRadius(jsonComponent.GetFloat("AttenuationRadius"));
						spotLight->SetDirection(actor->GetTransform().GetForward());
						spotLight->SetShadowMapSize({ 512, 512 });

						break;
					}
					case ComponentType::DirectionalLightComponent:
					{
						if (!jsonComponent.HasKey("Color"))
						{
							SPDLOG_ERROR("Directional Light has no Color [{}]!", actorName);
							break;
						}
						if (!jsonComponent.HasKey("Intensity"))
						{
							SPDLOG_ERROR("Directional Light has no Intensity [{}]!", actorName);
							break;
						}

						DirectionalLightComponent* directionalLight = actor->AddComponent<DirectionalLightComponent>();

						std::vector<float> jsonColor = jsonComponent.GetFloatArray("Color");
						CommonUtilities::Vector3f color = { jsonColor[0],jsonColor[1] ,jsonColor[2] };

						directionalLight->SetColor(color);
						directionalLight->SetIntensity(jsonComponent.GetFloat("Intensity"));
						directionalLight->SetDirection(actor->GetTransform().GetForward());

						break;
					}
					case ComponentType::BoxComponent:
					{
						break;
					}
					case ComponentType::SphereComponent:
					{
						break;
					}
					case ComponentType::CapsuleComponent:
					{
						break;
					}
					case ComponentType::SpringArmComponent:
					{
						break;
					}
					default:
						// Log error here
						break;
				}
			}

			myActors.push_back(std::move(actor));
		}
	}
	else
	{
		SPDLOG_WARN("No Actors found in scene [{}]. Scene might be empty", aPath.string());
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


