#pragma once

#include "MathUtils.hpp"
#include "Plane.hpp"
#include "Ray.hpp"
#include "Sphere.hpp"
#include "AABB3D.hpp"
#include "PlaneVolume.hpp"

namespace CommonUtilities
{
	struct IntersectionData 
	{
		Vector3f Point;
		Vector3f Normal;
		float Depth;
	};

	template <typename T>
	bool IntersectionPlaneRay(const Plane<T>& aPlane, const Ray<T>& aRay, IntersectionData* collisionDataOut = nullptr)
	{
		const Vector3<T>& rayOrigin = aRay.GetOrigin();
		const Vector3<T>& rayDirection = aRay.GetDirection();

		const Vector3<T>& planePoint = aPlane.GetPoint();
		const Vector3<T>& planeNormal = aPlane.GetNormal();

		const T distance = (rayOrigin - aPlane.GetPoint()).Dot(aPlane.GetNormal().GetNormalized());

		if (Math::Abs(distance) < T(1e-6))
		{
			if (collisionDataOut != nullptr)
			{
				collisionDataOut->Point = rayOrigin;
			}

			return true;
		}

		if (aRay.GetDirection() == Vector3<T>::Zero)
		{
			return false;
		}

		const T denominator = Vector3<T>::Dot(rayDirection, planeNormal);

		if (Math::Abs(denominator) < T(1e-6))
		{
			return false;
		}

		const T intersectionT = Vector3<T>::Dot(Vector3<T>(planePoint - rayOrigin), planeNormal) / denominator;

		if (intersectionT < T(1e-6))
		{
			return false;
		}

		if (collisionDataOut != nullptr)
		{
			collisionDataOut->Point = rayOrigin + (rayDirection * intersectionT);
		}

		return true;
	}

	template <typename T>
	bool IntersectionPlaneSphere(const Plane<T>& aPlane, const Sphere<T>& aSphere, IntersectionData* collisionDataOut = nullptr)
	{
		// TODO : Fill out info
		collisionDataOut;

		const Vector3<T> planeNormal = aPlane.GetNormal().GetNormalized();
		const Vector3<T> centerToPlane = aSphere.GetCenter() - aPlane.GetPoint();

		const T distance = Vector3<T>::Dot(planeNormal, centerToPlane);

		if (Math::Abs(distance) > aSphere.GetRadius())
		{
			return false;
		}

		return true;
	}

	// TODO : Implement collision method for AABB and Plane
	template <typename T>
	bool IntersectionPlaneAABB(const Plane<T>& aPlane, const AABB3D<T>& aAABB, IntersectionData* collisionDataOut = nullptr)
	{


		return true;
	}

	template<typename T>
	bool IntersectionAABBRay(const AABB3D<T>& aAABB, const Ray<T>& aRay, IntersectionData* collisionDataOut = nullptr)
	{
		const Vector3<T>& rayOrigin = aRay.GetOrigin();
		const Vector3<T> rayDirection = aRay.GetDirection().GetNormalized();

		const Vector3<T>& aabbMin = aAABB.GetMin();
		const Vector3<T>& aabbMax = aAABB.GetMax();

		if (aAABB.IsInside(rayOrigin))
		{
			if (collisionDataOut != nullptr)
			{
				collisionDataOut->Point = rayOrigin;
			}

			return true;
		}

		if (aRay.GetDirection() == Vector3<T>::Zero)
		{
			return false;
		}

		T xMinIntersect;
		T xMaxIntersect;

		if (rayDirection.x > 0)
		{
			xMinIntersect = (aabbMin.x - rayOrigin.x) * (static_cast<T>(1) / rayDirection.x);
			xMaxIntersect = (aabbMax.x - rayOrigin.x) * (static_cast<T>(1) / rayDirection.x);
		}
		else if (rayDirection.x < 0)
		{
			xMinIntersect = (aabbMax.x - rayOrigin.x) * (static_cast<T>(1) / rayDirection.x);
			xMaxIntersect = (aabbMin.x - rayOrigin.x) * (static_cast<T>(1) / rayDirection.x);
		}
		else
		{
			if (rayOrigin.x < aabbMin.x || rayOrigin.x > aabbMax.x)
			{
				return false;
			}

			xMinIntersect = -std::numeric_limits<T>::infinity();
			xMaxIntersect = std::numeric_limits<T>::infinity();
		}

		T yMinIntersect;
		T yMaxIntersect;

		if (rayDirection.y > 0)
		{
			yMinIntersect = (aabbMin.y - rayOrigin.y) * (static_cast<T>(1) / rayDirection.y);
			yMaxIntersect = (aabbMax.y - rayOrigin.y) * (static_cast<T>(1) / rayDirection.y);
		}
		else if (rayDirection.y < 0)
		{
			yMinIntersect = (aabbMax.y - rayOrigin.y) * (static_cast<T>(1) / rayDirection.y);
			yMaxIntersect = (aabbMin.y - rayOrigin.y) * (static_cast<T>(1) / rayDirection.y);
		}
		else
		{
			if (rayOrigin.y < aabbMin.y || rayOrigin.y > aabbMax.y)
			{
				return false;
			}

			yMinIntersect = -std::numeric_limits<T>::infinity();
			yMaxIntersect = std::numeric_limits<T>::infinity();
		}

		T zMinIntersect;
		T zMaxIntersect;

		if (rayDirection.z > 0)
		{
			zMinIntersect = (aabbMin.z - rayOrigin.z) * (static_cast<T>(1) / rayDirection.z);
			zMaxIntersect = (aabbMax.z - rayOrigin.z) * (static_cast<T>(1) / rayDirection.z);
		}
		else if (rayDirection.z < 0)
		{
			zMinIntersect = (aabbMax.z - rayOrigin.z) * (static_cast<T>(1) / rayDirection.z);
			zMaxIntersect = (aabbMin.z - rayOrigin.z) * (static_cast<T>(1) / rayDirection.z);
		}
		else
		{
			if (rayOrigin.z < aabbMin.z || rayOrigin.z > aabbMax.z)
			{
				return false;
			}

			zMinIntersect = -std::numeric_limits<T>::infinity();
			zMaxIntersect = std::numeric_limits<T>::infinity();
		}

		T tEntry = Math::Max(std::vector<T>{ xMinIntersect, yMinIntersect, zMinIntersect });
		T tExit = Math::Min(std::vector<T>{ xMaxIntersect, yMaxIntersect, zMaxIntersect });

		if (tEntry > tExit)
		{
			return false;
		}
		else if (tExit < 0)
		{
			return false;
		}

		if (collisionDataOut != nullptr)
		{
			collisionDataOut->Point = rayOrigin + rayDirection * tEntry;
		}

		return true;
	}

	template<typename T>
	bool IntersectionSphereRay(const Sphere<T>& aSphere, const Ray<T>& aRay, IntersectionData* collisionDataOut = nullptr)
	{
		const Vector3<T>& rayOrigin = aRay.GetOrigin();
		const Vector3<T>& rayDirection = aRay.GetDirection();

		const Vector3<T>& sphereCenter = aSphere.GetCenter();

		if (aSphere.IsInside(rayOrigin))
		{
			if (collisionDataOut != nullptr)
			{
				collisionDataOut->Point = rayOrigin;
			}

			return true;
		}

		if (aRay.GetDirection() == Vector3<T>::Zero)
		{
			return false;
		}

		const T projection = Vector3<T>::Dot(rayDirection, sphereCenter - rayOrigin);

		if (projection < 0)
		{
			return false;
		}

		const Vector3<T> point = rayOrigin + (rayDirection * projection);

		if (!aSphere.IsInside(point))
		{
			return false;
		}

		if (collisionDataOut != nullptr)
		{
			collisionDataOut->Point = point;
		}

		return true;
	}

	template<typename T>
	bool IntersectionSphereAABB(const Sphere<T>& aSphere, const AABB3D<T>& aAABB3D, IntersectionData* collisionDataOut = nullptr)
	{
		const Vector3<T> myClosestPoint = 
		{ 
			Math::Clamp(aSphere.GetCenter().x, aAABB3D.GetMin().x, aAABB3D.GetMax().x), 
			Math::Clamp(aSphere.GetCenter().y, aAABB3D.GetMin().y, aAABB3D.GetMax().y), 
			Math::Clamp(aSphere.GetCenter().z, aAABB3D.GetMin().z, aAABB3D.GetMax().z) 
		};

		if (!aSphere.IsInside(myClosestPoint))
		{
			return false;
		}

		if (collisionDataOut != nullptr)
		{
			collisionDataOut->Point = myClosestPoint;
		}

		return true;
	}

	template<typename T>
	bool IntersectionSphereSphere(const Sphere<T>& aSphere0, const Sphere<T>& aSphere1, IntersectionData* collisionDataOut = nullptr)
	{
		// TODO : Fill out info
		collisionDataOut;

		const T totalDistance = (aSphere1.GetCenter() - aSphere0.GetCenter()).LengthSqr();
		const T totalRadius = (aSphere0.GetRadius() + aSphere1.GetRadius()) * (aSphere0.GetRadius() + aSphere1.GetRadius());

		if (totalDistance > totalRadius)
		{
			return false;
		}

		return true;
	}

	template<typename T>
	bool IntersectionAABBAABB(const AABB3D<T>& aAABB0, const AABB3D<T>& aAABB1, IntersectionData* collisionDataOut = nullptr)
	{
		const Vector3<T>& min0 = aAABB0.GetMin();
		const Vector3<T>& max0 = aAABB0.GetMax();
		const Vector3<T>& min1 = aAABB1.GetMin();
		const Vector3<T>& max1 = aAABB1.GetMax();

		const Vector3<T> normals[6] =
		{
			{ -1.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, -1.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, -1.0f },
			{ 0.0f, 0.0f, 1.0f },
		};

		const T distances[6] =
		{
			max1.x - min0.x,
			max0.x - min1.x,
			max1.y - min0.y,
			max0.y - min1.y,
			max1.z - min0.z,
			max0.z - min1.z
		};

		int collidedFace = 0;

		for (int i = 0; i < 6; i++)
		{
			if (distances[i] < 0.0f)
			{
				return false;
			}

			const int axis = static_cast<int>(i * 0.5f);

			collidedFace = distances[i] < distances[collidedFace] ? i : collidedFace;
		}

		if (collisionDataOut != nullptr)
		{
			collisionDataOut->Normal = normals[collidedFace];
			collisionDataOut->Depth = distances[collidedFace];
		}

		return true;
	}

	template<typename T>
	bool IntersectionSpherePlaneVolume(const Sphere<T>& aSphere, const PlaneVolume<T>& aPlaneVolume)
	{
		for (const Plane<T>& plane : aPlaneVolume.GetPlanes())
		{
			const T distance = Vector3<T>::Dot(aSphere.GetCenter() - plane.GetPoint(), plane.GetNormal());

			if (distance < -aSphere.GetRadius())
			{
				return false;
			}
		}

		return true;
	}

	// TODO : Implement collision method for AABB and PlaneVolume
	template<typename T>
	bool IntersectionAABBPlaneVolume(const AABB3D<T>& aAABB, const PlaneVolume<T>& aPlaneVolume)
	{
		return true;
	}
}