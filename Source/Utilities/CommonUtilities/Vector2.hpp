#pragma once

#include <iostream>

#pragma warning(push)
#pragma warning(disable : 4201)

namespace Tga
{
	template <typename T>
	class Vector2;
}

namespace CommonUtilities
{
	template <typename T>
	class Vector3;
}

namespace CommonUtilities
{
	template <typename T>
	class Vector2
	{
		public:
			union
			{
				T values[2];

				struct
				{
					T x;
					T y;
				};
			};

			static Vector2<T> Zero;
			static Vector2<T> One;

			Vector2();
			Vector2(const T &aX, const T &aY);

			template <class TargetType>
			Vector2<TargetType> ToType() const;
			Tga::Vector2<T> ToTga() const;

			T LengthSqr() const;
			T Length() const;

			Vector2<T> GetNormalized() const;
			void Normalize();

			T Dot(const Vector2<T> &aVector) const;
			static T Dot(const Vector2<T>& aVector0, const Vector2<T>& aVector1);

			Vector3<T> ToXY0() const;
	};

	using Vector2f = Vector2<float>;
	using Vector2i = Vector2<int>;
	using Vector2ui = Vector2<unsigned int>;

	template<typename T>
	Vector2<T> Vector2<T>::Zero = Vector2<T>(static_cast<T>(0), static_cast<T>(0));

	template <typename T>
	Vector2<T> Vector2<T>::One = Vector2<T>(static_cast<T>(1), static_cast<T>(1));

#pragma region Operators

#pragma region Negation Operator

	template <typename T>
	Vector2<T> operator-(const Vector2<T>& aVector)
	{
		return Vector2<T>(-aVector.x, -aVector.y);
	}

#pragma endregion

#pragma region Addition Operators

	template <typename T>
	Vector2<T> operator+(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>(aVector0.x + aVector1.x, aVector0.y + aVector1.y);
	}

	template <typename T>
	void operator+=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0 = aVector0 + aVector1;
	}

#pragma endregion

#pragma region Subtraction Operators

	template <typename T>
	Vector2<T> operator-(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>(aVector0.x - aVector1.x, aVector0.y - aVector1.y);
	}

	template <typename T>
	void operator-=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		aVector0 = aVector0 - aVector1;
	}

#pragma endregion

#pragma region Multiplication Operators

	template <typename T>
	Vector2<T> operator*(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>(aVector0.x * aVector1.x, aVector0.y * aVector1.y);
	}

	template <typename T>
	Vector2<T> operator*(const Vector2<T>& aVector, const T aScalar)
	{
		return Vector2<T>(aVector.x * aScalar, aVector.y * aScalar);
	}

	template <typename T>
	Vector2<T> operator*(const T aScalar, const Vector2<T>& aVector)
	{
		return Vector2<T>(aVector.x * aScalar, aVector.y * aScalar);
	}

	template <typename T>
	void operator*=(Vector2<T>& aVector, const T aScalar)
	{
		aVector = aVector * aScalar;
	}

#pragma endregion

#pragma region Division Operators

	template <typename T>
	Vector2<T> operator/(const Vector2<T>& aVector, const T aScalar)
	{
		if constexpr (std::is_floating_point<T>::value)
		{
			T dividerToMultiplier = static_cast<T>(1) / aScalar;
			return { aVector.x * dividerToMultiplier, aVector.y * dividerToMultiplier };
		}
		else
		{
			return { aVector.x / aScalar, aVector.y / aScalar };
		}
	}

	template <typename T>
	void operator/=(Vector2<T>& aVector, const T& aScalar)
	{
		aVector = aVector / aScalar;
	}

#pragma endregion

#pragma region Comparision Operators

	template <typename T>
	bool operator==(const Vector2<T>& lhs, const Vector2<T>& rhs)
	{
		if (lhs.x == rhs.x && lhs.y == rhs.y)
		{
			return true;
		}

		return false;
	}

	template <typename T>
	bool operator!=(const Vector2<T>& lhs, const Vector2<T>& rhs)
	{
		if (!(lhs == rhs))
		{
			return true;
		}

		return false;
	}

#pragma endregion

#pragma region OStream Operator

	template<typename T>
	std::ostream& operator<<(std::ostream& aOStream, const Vector2<T>& aVector)
	{
		aOStream << aVector.x << ", " << aVector.y;

		return aOStream;
	}

#pragma endregion

#pragma endregion

#pragma region Constructors

	template <typename T>
	inline Vector2<T>::Vector2()
	{
		x = 0;
		y = 0;	
	}

	template <typename T>
	inline Vector2<T>::Vector2(const T &aX, const T &aY)
	{
		x = aX;
		y = aY;
	}

#pragma endregion

	template <typename T>
	template <class TargetType>
	inline Vector2<TargetType> Vector2<T>::ToType() const
	{
		Vector2<TargetType> targetVector = 
		{
			static_cast<TargetType>(x), 
			static_cast<TargetType>(y)
		};

		return targetVector;
	}

	template <typename T>
	inline Tga::Vector2<T> Vector2<T>::ToTga() const
	{
		Tga::Vector2<T> tgaVector = 
		{
			static_cast<T>(x), 
			static_cast<T>(y)
		};

		return tgaVector;
	}

	template <typename T>
	inline T Vector2<T>::LengthSqr() const
	{
		return x * x + y * y;
	}

	template <typename T>
	inline T Vector2<T>::Length() const
	{
		return static_cast<T>(std::sqrt(LengthSqr()));
	}

	template <typename T>
	inline Vector2<T> Vector2<T>::GetNormalized() const
	{
		Vector2<T> normalizedVector = *this;
		normalizedVector.Normalize();

		return normalizedVector;
	}

	template <typename T>
	inline void Vector2<T>::Normalize()
	{
		T length = Length();

		if (length == T(0))
		{
			return;
		}

		x *= (1 / length);
		y *= (1 / length);
	}

	template <typename T>
	inline T Vector2<T>::Dot(const Vector2<T>& aVector) const
	{
		return (x * aVector.x) + (y * aVector.y);
	}

	template<typename T>
	inline T Vector2<T>::Dot(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return (aVector0.x * aVector1.x) + (aVector0.y * aVector1.y);
	}

	template<typename T>
	inline Vector3<T> Vector2<T>::ToXY0() const
	{
		return Vector3<T>(x, y, 0);
	}
}

#pragma warning(pop)