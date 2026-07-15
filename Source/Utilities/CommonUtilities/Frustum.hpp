#pragma once

#include <cassert>
#include <vector>

#include "Sphere.hpp"
#include "AABB3D.hpp"
#include "Matrix4x4.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "PlaneVolume.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)

namespace CommonUtilities
{
	enum class FrustumCorner
	{
		NearTopLeft,
		NearTopRight,
		NearBottomLeft,
		NearBottomRight,
		FarTopLeft,
		FarTopRight,
		FarBottomLeft,
		FarBottomRight
	};

	template <typename T>
	class Frustum
	{
	public:
		union 
		{
			PlaneVolume<T> planeVolume;

			struct
			{
				Plane<T> nearPlane;
				Plane<T> farPlane;
				Plane<T> leftPlane;
				Plane<T> rightPlane;
				Plane<T> topPlane;
				Plane<T> bottomPlane;
			};
		};

		Frustum();
		Frustum(T aFov, T aNearPlane, T aFarPlane, T aAspectRatio);
		~Frustum();

		Frustum(const Frustum& aOther);
		Frustum<T> operator=(const Frustum& aOther);

		T GetFov() const;
		T GetAspectRatio() const;

		void BuildFrustum(const Matrix4x4<T>& aCameraTransform);

		std::vector<Vector3<T>> GetCorners(const Matrix4x4<T>& aCameraViewMatrix, const Matrix4x4<T>& aCameraProjection) const;
		CommonUtilities::Vector3<T> GetCenter(const Matrix4x4<T>& aCameraViewMatrix, const Matrix4x4<T>& aCameraProjection) const;

		bool IsInside(const Vector3<T>& aPoint);
		bool IsInside(const Sphere<T>& aSphere);
		bool IsInside(const AABB3D<T>& aAABB);

	private:
		T myFov;
		T myZNear;
		T myZFar;
		T myAspectRatio;
	};


	template<typename T>
	inline Frustum<T>::Frustum()
	{
		myFov = 0;
		myZNear = 0;
		myZFar = 0;
		myAspectRatio = 0;

		nearPlane = Plane<T>();
		farPlane = Plane<T>();
		rightPlane = Plane<T>();
		leftPlane = Plane<T>();
		topPlane = Plane<T>();
		bottomPlane = Plane<T>();
	}

	template<typename T>
	inline Frustum<T>::Frustum(T aFov, T aNearPlane, T aFarPlane, T aAspectRatio)
	{
		assert(aNearPlane > static_cast<T>(0) && aNearPlane < aFarPlane);
		assert(aAspectRatio > static_cast<T>(0));

		myFov = aFov * (T(3.14159265358979323846) / T(180));
		myZNear = aNearPlane;
		myZFar = aFarPlane;
		myAspectRatio = aAspectRatio;

		nearPlane = Plane<T>();
		farPlane = Plane<T>();
		rightPlane = Plane<T>();
		leftPlane = Plane<T>();
		topPlane = Plane<T>();
		bottomPlane = Plane<T>();
	}

	template<typename T>
	inline Frustum<T>::~Frustum()
	{}

	template<typename T>
	inline Frustum<T>::Frustum(const Frustum& aOther)
	{
		nearPlane = aOther.nearPlane;
		farPlane = aOther.farPlane;
		rightPlane = aOther.rightPlane;
		leftPlane = aOther.leftPlane;
		topPlane = aOther.topPlane;
		bottomPlane = aOther.bottomPlane;
		myFov = aOther.myFov;
		myZNear = aOther.myZNear;
		myZFar = aOther.myZFar;
		myAspectRatio = aOther.myAspectRatio;
	}

	template<typename T>
	inline Frustum<T> Frustum<T>::operator=(const Frustum& aOther)
	{
		if (this != &aOther)
		{
			nearPlane = aOther.nearPlane;
			farPlane = aOther.farPlane;
			rightPlane = aOther.rightPlane;
			leftPlane = aOther.leftPlane;
			topPlane = aOther.topPlane;
			bottomPlane = aOther.bottomPlane;
			myFov = aOther.myFov;
			myZNear = aOther.myZNear;
			myZFar = aOther.myZFar;
			myAspectRatio = aOther.myAspectRatio;
		}

		return *this;
	}

	template<typename T>
	inline T Frustum<T>::GetFov() const
	{
		return myFov;
	}

	template<typename T>
	inline T Frustum<T>::GetAspectRatio() const
	{
		return myAspectRatio;
	}

	template<typename T>
	inline void Frustum<T>::BuildFrustum(const Matrix4x4<T>& aCameraTransform)
	{
		const Vector3<T> position = aCameraTransform.GetTranslation();
		const Vector3<T> forward = aCameraTransform.GetForward();
		const Vector3<T> up = aCameraTransform.GetUp();
		const Vector3<T> right = aCameraTransform.GetRight();

		const T farPlaneHalfHeight = myZFar * std::tan(myFov * static_cast<T>(0.5));
		const T farPlaneHalfWidth = farPlaneHalfHeight * myAspectRatio;
		const Vector3<T> frontMultFar = myZFar * forward;

		nearPlane	= Plane<T>(position + forward * myZNear, forward);
		farPlane	= Plane<T>(position + forward * myZFar, -forward);
		rightPlane	= Plane<T>(position, Vector3<T>::Cross(frontMultFar - right * farPlaneHalfWidth, up));
		leftPlane	= Plane<T>(position, Vector3<T>::Cross(up, frontMultFar + right * farPlaneHalfWidth));
		topPlane	= Plane<T>(position, Vector3<T>::Cross(right, frontMultFar - up * farPlaneHalfHeight));
		bottomPlane = Plane<T>(position, Vector3<T>::Cross(frontMultFar + up * farPlaneHalfHeight, right));
	}

	template<typename T>
	inline std::vector<Vector3<T>> Frustum<T>::GetCorners(const Matrix4x4<T>& aCameraViewMatrix, const Matrix4x4<T>& aCameraProjection) const
	{
		std::vector<Vector4<T>> vector4Corners(8);

		vector4Corners[static_cast<int>(FrustumCorner::NearTopLeft)]	 = Vector4<T>(-1, 1, 0, 1);
		vector4Corners[static_cast<int>(FrustumCorner::NearTopRight)]	 = Vector4<T>( 1, 1, 0, 1);
		vector4Corners[static_cast<int>(FrustumCorner::NearBottomLeft)]  = Vector4<T>(-1,-1, 0, 1);
		vector4Corners[static_cast<int>(FrustumCorner::NearBottomRight)] = Vector4<T>( 1,-1, 0, 1);
		vector4Corners[static_cast<int>(FrustumCorner::FarTopLeft)]		 = Vector4<T>(-1, 1, 1, 1);
		vector4Corners[static_cast<int>(FrustumCorner::FarTopRight)]	 = Vector4<T>( 1, 1, 1, 1);
		vector4Corners[static_cast<int>(FrustumCorner::FarBottomLeft)]	 = Vector4<T>(-1,-1, 1, 1);
		vector4Corners[static_cast<int>(FrustumCorner::FarBottomRight)]  = Vector4<T>( 1,-1, 1, 1);

		Matrix4x4<T> inverseViewProjection = Matrix4x4<T>(aCameraViewMatrix * aCameraProjection).GetInverse();

		vector4Corners[static_cast<int>(FrustumCorner::NearTopLeft)]	 *= inverseViewProjection;
		vector4Corners[static_cast<int>(FrustumCorner::NearTopRight)]	 *= inverseViewProjection;
		vector4Corners[static_cast<int>(FrustumCorner::NearBottomLeft)]  *= inverseViewProjection;
		vector4Corners[static_cast<int>(FrustumCorner::NearBottomRight)] *= inverseViewProjection;
		vector4Corners[static_cast<int>(FrustumCorner::FarTopLeft)]		 *= inverseViewProjection;
		vector4Corners[static_cast<int>(FrustumCorner::FarTopRight)]	 *= inverseViewProjection;
		vector4Corners[static_cast<int>(FrustumCorner::FarBottomLeft)]	 *= inverseViewProjection;
		vector4Corners[static_cast<int>(FrustumCorner::FarBottomRight)]  *= inverseViewProjection;

		vector4Corners[static_cast<int>(FrustumCorner::NearTopLeft)]	 /= vector4Corners[static_cast<int>(FrustumCorner::NearTopLeft)].w;
		vector4Corners[static_cast<int>(FrustumCorner::NearTopRight)]	 /= vector4Corners[static_cast<int>(FrustumCorner::NearTopRight)].w;
		vector4Corners[static_cast<int>(FrustumCorner::NearBottomLeft)]  /= vector4Corners[static_cast<int>(FrustumCorner::NearBottomLeft)].w;
		vector4Corners[static_cast<int>(FrustumCorner::NearBottomRight)] /= vector4Corners[static_cast<int>(FrustumCorner::NearBottomRight)].w;
		vector4Corners[static_cast<int>(FrustumCorner::FarTopLeft)]		 /= vector4Corners[static_cast<int>(FrustumCorner::FarTopLeft)].w;
		vector4Corners[static_cast<int>(FrustumCorner::FarTopRight)]	 /= vector4Corners[static_cast<int>(FrustumCorner::FarTopRight)].w;
		vector4Corners[static_cast<int>(FrustumCorner::FarBottomLeft)]	 /= vector4Corners[static_cast<int>(FrustumCorner::FarBottomLeft)].w;
		vector4Corners[static_cast<int>(FrustumCorner::FarBottomRight)]  /= vector4Corners[static_cast<int>(FrustumCorner::FarBottomRight)].w;

		std::vector<Vector3<T>> corners(8);

		for (int c = 0; c < 8; c++)
		{
			corners[c] = Vector3<T>(vector4Corners[c].x, vector4Corners[c].y, vector4Corners[c].z);
		}

		return corners;
	}

	template<typename T>
	inline CommonUtilities::Vector3<T> Frustum<T>::GetCenter(const Matrix4x4<T>& aCameraViewMatrix, const Matrix4x4<T>& aCameraProjection) const
	{
		std::vector<Vector3<T>> corners = GetCorners(aCameraViewMatrix, aCameraProjection);

		Vector3<T> center;

		for (const Vector3<T>& corner : corners)
		{
			center += corner;
		}

		center /= static_cast<T>(corners.size());

		return center;
	}

	template<typename T>
	inline bool Frustum<T>::IsInside(const Vector3<T>& aPoint)
	{
		return planeVolume.IsInside(aPoint);
	}

	template<typename T>
	inline bool Frustum<T>::IsInside(const Sphere<T>& aSphere)
	{
		for (const Plane<T>& plane : planeVolume)
		{
			const T distance = Vector3<T>::Dot(aSphere.GetCenter() - plane.GetPoint(), plane.GetNormal());

			if (distance < -aSphere.GetRadius())
			{
				return false;
			}
		}

		return true;
	}

	// TODO: Implement this function to check if an AABB is inside the frustum
	template<typename T>
	inline bool Frustum<T>::IsInside(const AABB3D<T>& aAABB)
	{
		return false;
	}
}

#pragma warning(pop)