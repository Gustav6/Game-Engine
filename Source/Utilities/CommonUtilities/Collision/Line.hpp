#pragma once

#include "Vector2.hpp"

namespace CommonUtilities
{
	template <typename T>
	class Line
	{
		public:
			Line();
			Line(const Vector2<T> &aPoint0, const Vector2<T> &aPoint1);

			void InitWith2Points(const Vector2<T> &aPoint0, const Vector2<T> &aPoint1);
			void InitWithPointAndDirection(const Vector2<T> &aPoint, const Vector2<T> &aDirection);
			void InitWithPointAndNormal(const Vector2<T> &aPoint, const Vector2<T> &aNormal);

			bool IsInside(const Vector2<T> &aPosition) const;

			const Vector2<T>& GetDirection() const;
			const Vector2<T> GetNormal() const;

		private:
			Vector2<T> myPoint;
			Vector2<T> myDirection;
	};

	template<typename T>
	inline Line<T>::Line()
	{
		myPoint = Vector2<T>::Zero;
		myDirection = Vector2<T>::Zero;
	}

	template<typename T>
	inline Line<T>::Line(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1)
	{
		myPoint = aPoint0;
		myDirection = aPoint1 - aPoint0;
	}

	template<typename T>
	inline void Line<T>::InitWith2Points(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1)
	{
		myPoint = aPoint0;
		myDirection = aPoint1 - aPoint0;
	}

	template<typename T>
	inline void Line<T>::InitWithPointAndDirection(const Vector2<T>& aPoint, const Vector2<T>& aDirection)
	{
		myPoint = aPoint;
		myDirection = aDirection;
	}

	template<typename T>
	inline void Line<T>::InitWithPointAndNormal(const Vector2<T>& aPoint, const Vector2<T>& aNormal)
	{
		myPoint = aPoint;
		myDirection = Vector2<T>(aNormal.y, -aNormal.x);
	}

	template<typename T>
	inline bool Line<T>::IsInside(const Vector2<T>& aPosition) const
	{
		if (myDirection != Vector2<T>::Zero)
		{
			if (GetNormal().Dot(aPosition - myPoint) <= 0)
			{
				return true;
			}
		}

		return false;
	}

	template<typename T>
	inline const Vector2<T>& Line<T>::GetDirection() const
	{
		return myDirection;
	}

	template<typename T>
	inline const Vector2<T> Line<T>::GetNormal() const
	{
		return Vector2<T>(-myDirection.y, myDirection.x);
	}
}