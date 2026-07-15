#include "LightComponent.h"

#include <string>

#include "GraphicsEngine/GraphicsEngine.hpp"

#include "GraphicsEngine/Objects/Light.h"
#include "GraphicsEngine/Objects/Texture.h"

#include "ensure.h"

LightComponent::LightComponent(GameObject* aOwner)
	: Component(aOwner)
{
	myData = std::make_unique<Light>();

	myShadowMap = nullptr;

	myTransformDirty = true;
}

LightComponent::~LightComponent() = default;

void LightComponent::UpdateLightViewProjection()
{
	if (myTransformDirty)
	{
		myData->LightViewProjectionTexture = myTransform.GetFastInverse() * myProjectionMatrix * CommonUtilities::Matrix4x4f::CreateNDCToTextureSpace();
		myTransformDirty = false;
	}
}

const Light* LightComponent::GetData() const
{
	return myData.get();
}

void LightComponent::SetPosition(const CommonUtilities::Vector3f& aPosition)
{
	myData->Position = aPosition;

	myTransform(4, 1) = myData->Position.x;
	myTransform(4, 2) = myData->Position.y;
	myTransform(4, 3) = myData->Position.z;

	myTransformDirty = true;
}

void LightComponent::SetColor(const CommonUtilities::Vector3f& aColor)
{
	myData->Color = aColor;
}

void LightComponent::SetIntensity(float aIntensity)
{
	myData->Intensity = aIntensity;
}

void LightComponent::SetShadowMapSize(const CommonUtilities::Vector2ui& aSize)
{
	if (myShadowMapSize.x != aSize.x || myShadowMapSize.y != aSize.y)
	{
		myShadowMapSize = aSize;

		if (myShadowMapSize.x == myShadowMapSize.y == 0)
		{
			myShadowMap = nullptr;
			return;
		}

		myShadowMap = std::make_unique<Texture>();

		std::string_view name;

		switch (myData->Type)
		{
			case LightType::Directional:
			{
				name = "Directional_SM";

				GraphicsEngine::Get().CreateShadowMap(name, myShadowMapSize.x, myShadowMapSize.y, *myShadowMap, false);
				break;
			}
			case LightType::Point:
			{
				name = "Point_SM";

				GraphicsEngine::Get().CreateShadowMap(name, myShadowMapSize.x, myShadowMapSize.y, *myShadowMap, true);
				break;
			}
			case LightType::Spot:
			{
				name = "Spot_SM";

				GraphicsEngine::Get().CreateShadowMap(name, myShadowMapSize.x, myShadowMapSize.y, *myShadowMap, false);
				break;
			}
		}

		myProjectionMatrix = CreateProjectionMatrix();

		myTransformDirty = true;
	}
}

const Texture* LightComponent::GetShadowMap() const
{
	return myShadowMap.get();
}

const CommonUtilities::Matrix4x4f& LightComponent::GetProjection() const
{
	return myProjectionMatrix;
}

const CommonUtilities::Matrix4x4f& LightComponent::GetTransform() const
{
	return myTransform;
}
