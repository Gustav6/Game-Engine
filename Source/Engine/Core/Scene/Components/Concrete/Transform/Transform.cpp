#include "Core/Core.pch.h"
#include "Transform.h"

#include <Core/Scene/World/Actor.hpp>

Transform::Transform(Actor* aOwner)
	: IComponent(aOwner)
{
	myLocalPosition = CommonUtilities::Vector3f::Zero;
	myLocalScale = CommonUtilities::Vector3f::One;
	myLocalRotation = CommonUtilities::Quaternionf::Identity;

	myIsDirty = true;
}

void Transform::Translate(const CommonUtilities::Vector3f& aDelta, bool aWorldSpace)
{
	if (aWorldSpace)
	{
		SetPosition(GetPosition() + aDelta);
	}
	else
	{
		SetLocalPosition(myLocalPosition + aDelta);
	}
}

void Transform::Rotate(const CommonUtilities::Quaternionf& aDelta, bool aWorldSpace)
{
	if (aWorldSpace)
	{
		SetRotation((aDelta * GetRotation()).GetNormalized());
	}
	else
	{
		SetLocalRotation((myLocalRotation * aDelta).GetNormalized());
	}
}

void Transform::Rotate(const CommonUtilities::Vector3f& aEulerDelta, bool aWorldSpace)
{
	CommonUtilities::Quaternionf delta(aEulerDelta);
	Rotate(delta, aWorldSpace);
}

CommonUtilities::Vector3f Transform::GetPosition() const
{
	CommonUtilities::Matrix4x4f worldMatrix = GetWorldMatrix();

	CommonUtilities::Vector3f worldPosition
	{
		worldMatrix(4, 1), 
		worldMatrix(4, 2), 
		worldMatrix(4, 3)
	};

	return worldPosition;
}

void Transform::SetPosition(const CommonUtilities::Vector3f& aPosition)
{
	CommonUtilities::Vector4f localPosition =
	{
		aPosition.x,
		aPosition.y,
		aPosition.z,
		1
	};

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		CommonUtilities::Matrix4x4f parentWorldMatrix = parent->GetComponent<Transform>()->GetWorldMatrix();

		CommonUtilities::Vector3f scale = parentWorldMatrix.GetScaling();

		CommonUtilities::Matrix4x4f inverseParentMatrix;

		bool hasUnitScale =
			std::abs(scale.x - 1.0f) < 0.0001f &&
			std::abs(scale.y - 1.0f) < 0.0001f &&
			std::abs(scale.z - 1.0f) < 0.0001f;

		if (hasUnitScale)
		{
			inverseParentMatrix = parentWorldMatrix.GetFastInverse();
		}
		else
		{
			inverseParentMatrix = parentWorldMatrix.GetGeneralInverse();
		}

		localPosition = localPosition * inverseParentMatrix;
	}

	SetLocalPosition(CommonUtilities::Vector3f(localPosition.x, localPosition.y, localPosition.z));
}

const CommonUtilities::Vector3f& Transform::GetLocalPosition() const
{
	return myLocalPosition;
}

void Transform::SetLocalPosition(const CommonUtilities::Vector3f& aLocalPosition)
{
	myLocalPosition = aLocalPosition;
	myIsDirty = true;
}

CommonUtilities::Quaternionf Transform::GetRotation() const
{
	CommonUtilities::Quaternionf worldRotation = myLocalRotation;

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		worldRotation = parent->GetComponent<Transform>()->GetRotation() * worldRotation;
		worldRotation.Normalize();
	}

	return worldRotation;
}

CommonUtilities::Vector3f Transform::GetEulerRotation() const
{
	CommonUtilities::Quaternionf worldRotation = myLocalRotation;

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		worldRotation = parent->GetComponent<Transform>()->GetRotation() * worldRotation;
		worldRotation.Normalize();
	}

	return worldRotation.GetYawPitchRoll();
}

void Transform::SetRotation(const CommonUtilities::Quaternionf& aRotation)
{
	CommonUtilities::Quaternionf localRotation = aRotation;

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		localRotation = parent->GetComponent<Transform>()->GetRotation().GetConjugate() * localRotation;
	}

	SetLocalRotation(localRotation.GetNormalized());
}

void Transform::SetEulerRotation(const CommonUtilities::Vector3f& aEulerRotation)
{
	CommonUtilities::Quaternionf pitch({ 1, 0, 0 }, aEulerRotation.x);
	CommonUtilities::Quaternionf yaw({ 0, 1, 0 }, aEulerRotation.y);
	CommonUtilities::Quaternionf roll({ 0, 0, 1 }, aEulerRotation.z);

	CommonUtilities::Quaternionf localRotation = CommonUtilities::Quaternionf::Identity;

	localRotation = yaw * localRotation;
	localRotation = localRotation * pitch;
	localRotation = localRotation * roll;

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		localRotation = parent->GetComponent<Transform>()->GetRotation().GetConjugate() * localRotation;
	}

	SetLocalRotation(localRotation.GetNormalized());
}

const CommonUtilities::Quaternionf& Transform::GetLocalRotation() const
{
	return myLocalRotation;
}

const CommonUtilities::Vector3f Transform::GetLocalEulerRotation() const
{
	return myLocalRotation.GetYawPitchRoll();
}

void Transform::SetLocalRotation(const CommonUtilities::Quaternionf& aRotation)
{
	myLocalRotation = aRotation;
	myIsDirty = true;
}

void Transform::SetLocalEulerRotation(const CommonUtilities::Vector3f& aEulerRotation)
{
	CommonUtilities::Quaternionf localRotation(aEulerRotation);
	SetLocalRotation(localRotation);
}

CommonUtilities::Vector3f Transform::GetScale() const
{
	CommonUtilities::Vector3f worldScale = myLocalScale;

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		CommonUtilities::Vector3f parentScale = parent->GetComponent<Transform>()->GetScale();

		worldScale.x *= parentScale.x;
		worldScale.y *= parentScale.y;
		worldScale.z *= parentScale.z;
	}

	return worldScale;
}

void Transform::SetScale(const CommonUtilities::Vector3f& aScale)
{
	CommonUtilities::Vector3f localScale = aScale;

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		CommonUtilities::Vector3f parentScale = parent->GetComponent<Transform>()->GetScale();

		if (std::abs(parentScale.x) < 0.0001f)
		{
			parentScale.x = 0.0001f;
		}
		if (std::abs(parentScale.y) < 0.0001f)
		{
			parentScale.y = 0.0001f;
		}
		if (std::abs(parentScale.z) < 0.0001f)
		{
			parentScale.z = 0.0001f;
		}

		localScale.x /= parentScale.x;
		localScale.y /= parentScale.y;
		localScale.z /= parentScale.z;
	}

	SetLocalScale(localScale);
}

const CommonUtilities::Vector3f& Transform::GetLocalScale() const
{
	return myLocalScale;
}

void Transform::SetLocalScale(const CommonUtilities::Vector3f& aLocalScale)
{
	myLocalScale = aLocalScale;
	myIsDirty = true;
}

CommonUtilities::Vector3f Transform::GetRight() const
{
	CommonUtilities::Vector3f right = { 1.f, 0.f, 0.f };

	return CommonUtilities::Quaternionf::RotateVectorByQuaternion(GetRotation(), right);
}

CommonUtilities::Vector3f Transform::GetUp() const
{
	CommonUtilities::Vector3f up = { 0.f, 1.f, 0.f };

	return CommonUtilities::Quaternionf::RotateVectorByQuaternion(GetRotation(), up);
}

CommonUtilities::Vector3f Transform::GetForward() const
{
	CommonUtilities::Vector3f forward = { 0.f, 0.f, 1.f };

	return CommonUtilities::Quaternionf::RotateVectorByQuaternion(GetRotation(), forward);
}

CommonUtilities::Matrix4x4f Transform::GetWorldMatrix() const
{
	CommonUtilities::Matrix4x4f worldMatrix = GetLocalMatrix();

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		worldMatrix = worldMatrix * parent->GetComponent<Transform>()->GetWorldMatrix();
	}

	return worldMatrix;
}

const CommonUtilities::Matrix4x4f& Transform::GetLocalMatrix() const
{
	if (myIsDirty)
	{
		CommonUtilities::Matrix4x4f scale;

		scale(1, 1) = myLocalScale.x;
		scale(2, 2) = myLocalScale.y;
		scale(3, 3) = myLocalScale.z;

		CommonUtilities::Matrix4x4f rotation;

		const float x = myLocalRotation.x;
		const float y = myLocalRotation.y;
		const float z = myLocalRotation.z;
		const float w = myLocalRotation.w;

		rotation(1, 1) = 1 - 2 * (y * y + z * z);
		rotation(2, 1) = 2 * (x * y - w * z);
		rotation(3, 1) = 2 * (x * z + w * y);

		rotation(1, 2) = 2 * (x * y + w * z);
		rotation(2, 2) = 1 - 2 * (x * x + z * z);
		rotation(3, 2) = 2 * (y * z - w * x);

		rotation(1, 3) = 2 * (x * z - w * y);
		rotation(2, 3) = 2 * (y * z + w * x);
		rotation(3, 3) = 1 - 2 * (x * x + y * y);

		CommonUtilities::Matrix4x4f translation;

		translation(4, 1) = myLocalPosition.x;
		translation(4, 2) = myLocalPosition.y;
		translation(4, 3) = myLocalPosition.z;

		myLocalMatrix = scale * rotation * translation;

		myIsDirty = false;
	}

	return myLocalMatrix;
}

void Transform::SetLocalMatrix(const CommonUtilities::Matrix4x4f& aLocalMatrix)
{
	myLocalMatrix = aLocalMatrix;

	myLocalPosition = { aLocalMatrix(4, 1), aLocalMatrix(4, 2), aLocalMatrix(4, 3) };

	CommonUtilities::Vector3f right = { aLocalMatrix(1, 1), aLocalMatrix(1, 2), aLocalMatrix(1, 3) };
	CommonUtilities::Vector3f up = { aLocalMatrix(2, 1), aLocalMatrix(2, 2), aLocalMatrix(2, 3) };
	CommonUtilities::Vector3f forward = { aLocalMatrix(3, 1), aLocalMatrix(3, 2), aLocalMatrix(3, 3) };

	myLocalScale.x = right.Length();
	myLocalScale.y = up.Length();
	myLocalScale.z = forward.Length();

	right.Normalize();
	up.Normalize();
	forward.Normalize();

	CommonUtilities::Matrix3x3f rotation;
	rotation(1, 1) = right.x;
	rotation(1, 2) = right.y;
	rotation(1, 3) = right.z;

	rotation(2, 1) = up.x;
	rotation(2, 2) = up.y;
	rotation(2, 3) = up.z;

	rotation(3, 1) = forward.x;
	rotation(3, 2) = forward.y;
	rotation(3, 3) = forward.z;

	myLocalRotation = CommonUtilities::Quaternionf(rotation);

	myIsDirty = false;
}

void Transform::SetWorldMatrix(const CommonUtilities::Matrix4x4f& aWorldMatrix)
{
	CommonUtilities::Matrix4x4f localMatrix = aWorldMatrix;

	Actor* parent = myOwner->GetParent();

	if (parent != nullptr)
	{
		Transform* parentTransform = parent->GetComponent<Transform>();

		CommonUtilities::Matrix4x4f parentWorldInverse = parentTransform->GetWorldMatrix().GetGeneralInverse();

		localMatrix = aWorldMatrix * parentWorldInverse;
	}

	SetLocalMatrix(localMatrix);
}