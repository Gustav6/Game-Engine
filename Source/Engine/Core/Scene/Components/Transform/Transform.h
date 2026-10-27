#pragma once

#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Quaternion.hpp"
#include "CommonUtilities/Math/Matrix4x4.hpp"

class Transform
{
public:
	Transform();
	Transform(const CommonUtilities::Matrix4x4f& aMatrix);

	void Translate(const CommonUtilities::Vector3f& aDelta);

	void Rotate(const CommonUtilities::Quaternionf& aDelta, bool aWorldSpace = false);
	void Rotate(const CommonUtilities::Vector3f& aEulerDelta, bool aWorldSpace = false);

	CommonUtilities::Vector3f GetPosition() const;
	void SetPosition(const CommonUtilities::Vector3f& aPosition);

	const CommonUtilities::Vector3f& GetLocalPosition() const;
	void SetLocalPosition(const CommonUtilities::Vector3f& aLocalPosition);

	CommonUtilities::Quaternionf GetRotation() const;
	CommonUtilities::Vector3f GetEulerRotation() const;
	void SetRotation(const CommonUtilities::Quaternionf& aRotation);
	void SetEulerRotation(const CommonUtilities::Vector3f& aEulerRotation);

	const CommonUtilities::Quaternionf& GetLocalRotation() const;
	const CommonUtilities::Vector3f GetLocalEulerRotation() const;
	void SetLocalRotation(const CommonUtilities::Quaternionf& aRotation);
	void SetLocalEulerRotation(const CommonUtilities::Vector3f& aEulerRotation);

	CommonUtilities::Vector3f GetScale() const;
	void SetScale(const CommonUtilities::Vector3f& aScale);

	const CommonUtilities::Vector3f& GetLocalScale() const;
	void SetLocalScale(const CommonUtilities::Vector3f& aLocalScale);

	CommonUtilities::Vector3f GetRight() const;
	CommonUtilities::Vector3f GetUp() const;
	CommonUtilities::Vector3f GetForward() const;

	CommonUtilities::Matrix4x4f GetWorldMatrix() const;
	const CommonUtilities::Matrix4x4f& GetLocalMatrix() const;

	void LookAt(const CommonUtilities::Vector3<float>& aTarget);

private:
	mutable bool myIsDirty;

	CommonUtilities::Vector3f myLocalPosition;
	CommonUtilities::Vector3f myLocalScale;
	CommonUtilities::Quaternionf myLocalRotation;
		
	mutable CommonUtilities::Matrix4x4f myLocalMatrix;
};