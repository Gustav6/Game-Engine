#pragma once

#include <cassert>
#include <vector>

#include "Sphere.hpp"
#include "AABB3D.hpp"
#include "Matrix4x4.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "PlaneVolume.hpp"
#include "MathUtils.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)

namespace CommonUtilities
{
	template <typename T>
	class Frustum
	{
	public:
		enum Corner_
		{
			Corner_NearTopLeft,
			Corner_NearTopRight,
			Corner_NearBottomLeft,
			Corner_NearBottomRight,
			Corner_FarTopLeft,
			Corner_FarTopRight,
			Corner_FarBottomLeft,
			Corner_FarBottomRight
		};

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

		std::vector<Vector4<T>> GetCornersInWorldSpace(const Matrix4x4<T>& aInverseViewProjection) const;
		
		Vector3<T> GetCenterInWorldSpace(const Matrix4x4<T>& aInverseViewProjection) const;

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

		myFov = aFov * Math::DEG_TO_RAD<T>;
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
	inline std::vector<Vector4<T>> Frustum<T>::GetCornersInWorldSpace(const Matrix4x4<T>& aInverseViewProjection) const
	{
		std::vector<Vector4<T>> corners(8);

		corners[static_cast<int>(Corner_NearTopLeft)]	  = Vector4<T>(-1.0f,  1.0f, 0.0f, 1.0f);
		corners[static_cast<int>(Corner_NearTopRight)]	  = Vector4<T>( 1.0f,  1.0f, 0.0f, 1.0f);
		corners[static_cast<int>(Corner_NearBottomLeft)]  = Vector4<T>(-1.0f, -1.0f, 0.0f, 1.0f);
		corners[static_cast<int>(Corner_NearBottomRight)] = Vector4<T>( 1.0f, -1.0f, 0.0f, 1.0f);
		corners[static_cast<int>(Corner_FarTopLeft)]	  = Vector4<T>(-1.0f,  1.0f, 1.0f, 1.0f);
		corners[static_cast<int>(Corner_FarTopRight)]	  = Vector4<T>( 1.0f,  1.0f, 1.0f, 1.0f);
		corners[static_cast<int>(Corner_FarBottomLeft)]	  = Vector4<T>(-1.0f, -1.0f, 1.0f, 1.0f);
		corners[static_cast<int>(Corner_FarBottomRight)]  = Vector4<T>( 1.0f, -1.0f, 1.0f, 1.0f);

		corners[static_cast<int>(Corner_NearTopLeft)]	  *= aInverseViewProjection;
		corners[static_cast<int>(Corner_NearTopRight)]	  *= aInverseViewProjection;
		corners[static_cast<int>(Corner_NearBottomLeft)]  *= aInverseViewProjection;
		corners[static_cast<int>(Corner_NearBottomRight)] *= aInverseViewProjection;
		corners[static_cast<int>(Corner_FarTopLeft)]	  *= aInverseViewProjection;
		corners[static_cast<int>(Corner_FarTopRight)]	  *= aInverseViewProjection;
		corners[static_cast<int>(Corner_FarBottomLeft)]	  *= aInverseViewProjection;
		corners[static_cast<int>(Corner_FarBottomRight)]  *= aInverseViewProjection;

		corners[static_cast<int>(Corner_NearTopLeft)]	  /= corners[static_cast<int>(Corner_NearTopLeft)].w;
		corners[static_cast<int>(Corner_NearTopRight)]	  /= corners[static_cast<int>(Corner_NearTopRight)].w;
		corners[static_cast<int>(Corner_NearBottomLeft)]  /= corners[static_cast<int>(Corner_NearBottomLeft)].w;
		corners[static_cast<int>(Corner_NearBottomRight)] /= corners[static_cast<int>(Corner_NearBottomRight)].w;
		corners[static_cast<int>(Corner_FarTopLeft)]	  /= corners[static_cast<int>(Corner_FarTopLeft)].w;
		corners[static_cast<int>(Corner_FarTopRight)]	  /= corners[static_cast<int>(Corner_FarTopRight)].w;
		corners[static_cast<int>(Corner_FarBottomLeft)]	  /= corners[static_cast<int>(Corner_FarBottomLeft)].w;
		corners[static_cast<int>(Corner_FarBottomRight)]  /= corners[static_cast<int>(Corner_FarBottomRight)].w;

		return corners;
	}

	template<typename T>
	inline CommonUtilities::Vector3<T> Frustum<T>::GetCenterInWorldSpace(const Matrix4x4<T>& aInverseViewProjection) const
	{
		std::vector<Vector4<T>> corners = GetCornersInWorldSpace(aInverseViewProjection);

		Vector4<T> center4;

		for (size_t c = 0; c < corners.size(); c++)
		{
			center4 += corners[c];
		}

		center4 /= T(8);

		Vector3<T> center = { center4.x, center4.y, center4.z };

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