#include "SpotLightComponent.h"

#include <cassert>

#include "CommonUtilities/Vector4.hpp"
#include "CommonUtilities/MathUtils.hpp"

#include "GraphicsEngine/Objects/Light.h"

SpotLightComponent::SpotLightComponent(GameObject* aOwner)
	: LightComponent(aOwner)
{
	myData->Type = LightType::Spot;
}

void SpotLightComponent::SetDirection(const CommonUtilities::Vector3f & aDirection)
{
	myData->Direction = aDirection.GetNormalized();

    CommonUtilities::Vector3f forward = myData->Direction;

    CommonUtilities::Vector3f worldUp = { 0.0f, 1.0f, 0.0f };

    if (abs(CommonUtilities::Vector3f::Dot(forward, worldUp)) > 0.99f)
    {
        worldUp = { 1.0f, 0.0f, 0.0f };
    }

    CommonUtilities::Vector3f right = CommonUtilities::Vector3f::Cross(worldUp, forward).GetNormalized();
    CommonUtilities::Vector3f up = CommonUtilities::Vector3f::Cross(forward, right);

    myTransform(1, 1) = right.x;
    myTransform(1, 2) = right.y;
    myTransform(1, 3) = right.z;

    myTransform(2, 1) = up.x;
    myTransform(2, 2) = up.y;
    myTransform(2, 3) = up.z;

    myTransform(3, 1) = forward.x;
    myTransform(3, 2) = forward.y;
    myTransform(3, 3) = forward.z;
}

void SpotLightComponent::SetRadius(float aRadius)
{
	myData->Radius = aRadius;
}

void SpotLightComponent::SetCone(float aInner, float aOuter)
{
	assert(aInner >= 0.0f && aInner <= 1.0f);
	assert(aOuter >= 0.0f && aOuter <= 1.0f);
	assert(aInner <= aOuter);

	myData->InnerCone = aInner;
	myData->OuterCone = aOuter;
}

const CommonUtilities::Matrix4x4f SpotLightComponent::CreateProjectionMatrix() const
{
    const float fov = acos(myData->OuterCone) * 2.0f * CommonUtilities::Math::RAD_TO_DEG<float>;

	return CommonUtilities::Matrix4x4f::CreatePerspectiveProjection(fov, 0.1f, myData->Radius, 1.0f);
}
