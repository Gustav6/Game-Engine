#pragma once

#include <cassert>
#include "Vector3.hpp"
#include "Quaternion.hpp"

namespace CommonUtilities
{
	template <typename T>
	class OBB3D
	{
		public:
			OBB3D();
			OBB3D(const Vector3<T>& aCenter, const Quaternion<T>& aRotation);

			void InitWithCenterExtentsAndRotation(const Vector3<T>& aCenter, const Quaternion<T>& aRotation);

			const Vector3<T>& GetCenter() const;
			const Quaternion<T>& GetRotation() const;

			// TODO: Implement
			bool IsInside(const Vector3<T>& aPoint) const;

		private:
			Vector3<T> myCenterPoint;
			Quaternion<T> myRotation;
	};

	template<typename T>
	inline OBB3D<T>::OBB3D()
	{
		myCenterPoint = Vector3<T>::Zero;
		myRotation = Quaternion<T>::Identity;
	}

	template<typename T>
	inline OBB3D<T>::OBB3D(const Vector3<T>& aCenter, const Quaternion<T>& aRotation)
	{
		myCenterPoint = aCenter;
		myRotation = aRotation;
	}

	template<typename T>
	inline void OBB3D<T>::InitWithCenterExtentsAndRotation(const Vector3<T>& aCenter, const Quaternion<T>& aRotation)
	{
		myCenterPoint = aCenter;
		myRotation = aRotation;
	}

	template<typename T>
	inline const Vector3<T>& OBB3D<T>::GetCenter() const
	{
		return myCenterPoint;
	}

	template<typename T>
	inline const Quaternion<T>& OBB3D<T>::GetRotation() const
	{
		return aRotation;
	}

	template<typename T>
	inline bool OBB3D<T>::IsInside(const Vector3<T>& aPoint) const
	{
		return false;
	}
}