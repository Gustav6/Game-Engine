#pragma once

#include "../Math/Vector3.hpp"

namespace CommonUtilities
{
	template <typename T>
	class Plane
	{
		public:
			Plane();
			Plane(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2);
			Plane(const Vector3<T>& aPoint0, const Vector3<T>& aNormal);

			Plane<T> operator=(const Plane& aOther);

			void InitWith3Points(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2);
			void InitWithPointAndNormal(const Vector3<T>& aPoint, const Vector3<T>& aNormal);

			bool IsInside(const Vector3<T>& aPosition) const;

			const Vector3<T>& GetNormal() const;
			const Vector3<T>& GetPoint() const;

		private:
			Vector3<T> myPoint;
			Vector3<T> myNormal;
	};

	template<typename T>
	inline CommonUtilities::Plane<T>::Plane()
	{
		myPoint = Vector3<T>::Zero;
		myNormal = Vector3<T>::Zero;
	}

	template<typename T>
	inline CommonUtilities::Plane<T>::Plane(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2)
	{
		myPoint = aPoint0;
		myNormal = Vector3<T>(aPoint1 - aPoint0).Cross((aPoint2 - aPoint0));
	}

	template<typename T>
	inline CommonUtilities::Plane<T>::Plane(const Vector3<T>& aPoint0, const Vector3<T>& aNormal)
	{
		myPoint = aPoint0;
		myNormal = aNormal;
	}

	template<typename T>
	inline Plane<T> Plane<T>::operator=(const Plane& aOther)
	{
		if (this != &aOther)
		{
			myPoint = aOther.myPoint;
			myNormal = aOther.myNormal;
		}

		return *this;
	}

	template<typename T>
	inline void CommonUtilities::Plane<T>::InitWith3Points(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2)
	{
		myPoint = aPoint0;
		myNormal = Vector3<T>(aPoint1 - aPoint0).Cross((aPoint2 - aPoint0));
	}

	template<typename T>
	inline void CommonUtilities::Plane<T>::InitWithPointAndNormal(const Vector3<T>& aPoint, const Vector3<T>& aNormal)
	{
		myPoint = aPoint;
		myNormal = aNormal;
	}

	template<typename T>
	inline bool CommonUtilities::Plane<T>::IsInside(const Vector3<T>& aPosition) const
	{
		assert(myNormal != Vector3<T>::Zero);

		if (Vector3<T>::Dot(myNormal, aPosition - myPoint) <= 0)
		{
			return true;
		}

		return false;
	}

	template<typename T>
	inline const Vector3<T>& CommonUtilities::Plane<T>::GetNormal() const
	{
		return myNormal;
	}

	template <typename T>
	inline const Vector3<T> &Plane<T>::GetPoint() const
	{
		return myPoint;
	}
}
