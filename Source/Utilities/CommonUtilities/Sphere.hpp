#pragma once

#include <cassert>
#include "Vector3.hpp"

namespace CommonUtilities
{
	template <typename T>
	class Sphere
	{
		public:
			Sphere();
			Sphere(const Vector3<T>& aCenter, T aRadius);

			void InitWithCenterAndRadius(const Vector3<T>& aCenter, T aRadius);

			const Vector3<T>& GetCenter() const;
			const T& GetRadius() const;

			bool IsInside(const Vector3<T> &aPosition) const;

		private:
			Vector3<T> myCenter;
			T myRadius;
	};

	template<typename T>
	inline Sphere<T>::Sphere()
	{
		myCenter = Vector3<T>::Zero;
		myRadius = 0;
	}

	template<typename T>
	inline Sphere<T>::Sphere(const Vector3<T>& aCenter, T aRadius)
	{
		assert(aRadius != 0);

		myCenter = aCenter;
		myRadius = aRadius;
	}

	template<typename T>
	inline void Sphere<T>::InitWithCenterAndRadius(const Vector3<T>& aCenter, T aRadius)
	{
		assert(aRadius != 0);

		myCenter = aCenter;
		myRadius = aRadius;
	}

	template<typename T>
	inline const Vector3<T>& Sphere<T>::GetCenter() const
	{
		return myCenter;
	}

	template<typename T>
	inline const T& Sphere<T>::GetRadius() const
	{
		return myRadius;
	}

	template <typename T>
	inline bool Sphere<T>::IsInside(const Vector3<T> &aPosition) const
	{
		if (myCenter == Vector3<T>::Zero && myRadius == 0)
		{
			return false;
		}
		else if (Vector3<T>(myCenter - aPosition).LengthSqr() > (myRadius * myRadius))
		{
			return false;
		}

		return true;
	}
}