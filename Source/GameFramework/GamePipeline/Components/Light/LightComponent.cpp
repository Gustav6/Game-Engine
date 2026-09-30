#include "GameFramework.pch.h"
#include "LightComponent.h"

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "GraphicsEngine/Objects/Scene/Light.h"

#include "GraphicsEngine/Objects/GPU/Texture.h"

#include "ensure.h"

LightComponent::LightComponent(Actor* aOwner)
	: Component(aOwner)
{
	myShadowMap = nullptr;

	myTransformDirty = true;
}

LightComponent::~LightComponent() = default;

void LightComponent::UpdateLightViewProjection()
{
	if (myTransformDirty)
	{
		myData.LightViewProjectionTexture = myTransform.GetFastInverse() * myProjection * CommonUtilities::Matrix4x4f::CreateNDCToTextureSpace();
		myTransformDirty = false;
	}
}

const Light& LightComponent::GetData() const
{
	return myData;
}

void LightComponent::SetPosition(const CommonUtilities::Vector3f& aPosition)
{
	myData.Position = aPosition;

	myTransform(4, 1) = myData.Position.x;
	myTransform(4, 2) = myData.Position.y;
	myTransform(4, 3) = myData.Position.z;

	myTransformDirty = true;
}

void LightComponent::SetColor(const CommonUtilities::Vector3f& aColor)
{
	myData.Color = aColor;
}

void LightComponent::SetIntensity(float aIntensity)
{
	myData.Intensity = aIntensity;
}

void LightComponent::SetShadowMapSize(const CommonUtilities::Vector2ui& aSize)
{
	if (myData.ShadowMapSize.x != aSize.x || myData.ShadowMapSize.y != aSize.y)
	{
		myData.ShadowMapSize = aSize;

		if (myData.ShadowMapSize.x == myData.ShadowMapSize.y == 0)
		{
			myShadowMap = nullptr;
			return;
		}

		myShadowMap = std::make_unique<Texture>();

		std::string_view name;

		switch (myData.Type)
		{
			case LightType::Directional:
			{
				name = "Directional_SM";

				GraphicsEngine::Get().CreateShadowMap(name, myData.ShadowMapSize.x, myData.ShadowMapSize.y, *myShadowMap, false);
				break;
			}
			case LightType::Point:
			{
				name = "Point_SM";

				GraphicsEngine::Get().CreateShadowMap(name, myData.ShadowMapSize.x, myData.ShadowMapSize.y, *myShadowMap, true);
				break;
			}
			case LightType::Spot:
			{
				name = "Spot_SM";

				GraphicsEngine::Get().CreateShadowMap(name, myData.ShadowMapSize.x, myData.ShadowMapSize.y, *myShadowMap, false);
				break;
			}
		}

		myProjection = CreateProjectionMatrix();

		myTransformDirty = true;
	}
}

const Texture* LightComponent::GetShadowMap() const
{
	return myShadowMap.get();
}

const CommonUtilities::Matrix4x4f& LightComponent::GetProjection() const
{
	return myProjection;
}

const CommonUtilities::Matrix4x4f& LightComponent::GetTransform() const
{
	return myTransform;
}
