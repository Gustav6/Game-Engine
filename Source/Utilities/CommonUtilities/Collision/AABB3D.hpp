#pragma once

#include <cassert>
#include "../Math/Vector3.hpp"

namespace CommonUtilities
{
	template <typename T>
	class AABB3D
	{
		public:
			AABB3D();
			AABB3D(const Vector3<T>& aMin, const Vector3<T>& aMax);

			void InitWithMinAndMax(const Vector3<T>& aMin, const Vector3<T>& aMax);

			const Vector3<T>& GetMin() const;
			const Vector3<T>& GetMax() const;

			Vector3<T> GetCenter() const;

			bool IsInside(const Vector3<T> &aPosition) const;

		private:
			Vector3<T> myMinPoint;
			Vector3<T> myMaxPoint;
	};

	template<typename T>
	inline AABB3D<T>::AABB3D()
	{
		myMinPoint = Vector3<T>::Zero;
		myMaxPoint = Vector3<T>::Zero;
	}

	template<typename T>
	inline AABB3D<T>::AABB3D(const Vector3<T>& aMin, const Vector3<T>& aMax)
	{
		assert(aMin != aMax);

		myMinPoint = aMin;
		myMaxPoint = aMax;
	}

	template<typename T>
	inline void AABB3D<T>::InitWithMinAndMax(const Vector3<T>& aMin, const Vector3<T>& aMax)
	{
		assert(aMin != aMax);

		myMinPoint = aMin;
		myMaxPoint = aMax;
	}

	template<typename T>
	inline const Vector3<T>& AABB3D<T>::GetMin() const
	{
		return myMinPoint;
	}

	template<typename T>
	inline const Vector3<T>& AABB3D<T>::GetMax() const
	{
		return myMaxPoint;
	}

	template<typename T>
	inline Vector3<T> AABB3D<T>::GetCenter() const
	{
		return (myMinPoint + myMaxPoint) * static_cast<T>(0.5);
	}

	template <typename T>
	inline bool AABB3D<T>::IsInside(const Vector3<T> &aPosition) const
	{
		if (myMinPoint == Vector3<T>::Zero && myMaxPoint == Vector3<T>::Zero)
		{	
			return false;
		}
		else if (myMinPoint.x > aPosition.x || myMinPoint.y > aPosition.y || myMinPoint.z > aPosition.z)
		{
			return false;
		}
		else if (myMaxPoint.x < aPosition.x || myMaxPoint.y < aPosition.y || myMaxPoint.z < aPosition.z)
		{
			return false;
		}

		return true;
	}
}