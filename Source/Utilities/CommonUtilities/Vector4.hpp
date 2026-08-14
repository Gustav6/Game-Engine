#pragma once

#include <iostream>

#pragma warning(push)
#pragma warning(disable : 4201)

namespace Tga
{
	template <typename T>
	class Vector4;
}

namespace CommonUtilities
{
	template <typename T>
	class Vector2;
	template <typename T>
	class Vector3;
}

namespace CommonUtilities
{
	template <typename T>
	class Vector4
	{
		public:
			union
			{
				T values[4];

				struct
				{
					T x;
					T y;
					T z;
					T w;
				};
			};

			static Vector4<T> Zero;
			static Vector4<T> One;

			Vector4();
			Vector4(const T &aX, const T &aY, const T &aZ, const T &aW);

			template <class TargetType>
			Vector4<TargetType> ToType() const;
			Tga::Vector4<T> ToTga() const;

			T LengthSqr() const;
			T Length() const;

			Vector4<T> GetNormalized() const;
			void Normalize();

			T Dot(const Vector4<T> &aVector) const;
			static T Dot(const Vector4<T>& aVector0, const Vector4<T>& aVector1);

			Vector2<T> ToXY() const;
			Vector3<T> ToXYZ() const;
	};

	using Vector4f = Vector4<float>;
	using Vector4i = Vector4<int>;
	using Vector4ui = Vector4<unsigned int>;

	template<typename T>
	Vector4<T> Vector4<T>::Zero = Vector4<T>(static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));

	template <typename T>
	Vector4<T> Vector4<T>::One = Vector4<T>(static_cast<T>(1), static_cast<T>(1), static_cast<T>(1), static_cast<T>(1));

#pragma region Operators

#pragma region Negation Operator

	template <typename T>
	Vector4<T> operator-(const Vector4<T>& aVector)
	{
		return Vector4<T>(-aVector.x, -aVector.y, -aVector.z, -aVector.w);
	}

#pragma endregion

#pragma region Addition Operators

	template <typename T>
	Vector4<T> operator+(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return Vector4<T>(aVector0.x + aVector1.x, aVector0.y + aVector1.y, aVector0.z + aVector1.z, aVector0.w + aVector1.w);
	}

	template <typename T>
	void operator+=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0 = aVector0 + aVector1;
	}

#pragma endregion

#pragma region Subtraction Operators

	template <typename T>
	Vector4<T> operator-(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return Vector4<T>(aVector0.x - aVector1.x, aVector0.y - aVector1.y, aVector0.z - aVector1.z, aVector0.w - aVector1.w);
	}

	template <typename T>
	void operator-=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		aVector0 = aVector0 - aVector1;
	}

#pragma endregion

#pragma region Multiplication Operators

	template <typename T>
	Vector4<T> operator*(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return Vector4<T>(aVector0.x * aVector1.x, aVector0.y * aVector1.y, aVector0.z * aVector1.z, aVector0.w * aVector1.w);
	}

	template <typename T>
	Vector4<T> operator*(const Vector4<T>& aVector, const T aScalar)
	{
		return Vector4<T>(aVector.x * aScalar, aVector.y * aScalar, aVector.z * aScalar, aVector.w * aScalar);
	}

	template <typename T>
	Vector4<T> operator*(const T aScalar, const Vector4<T>& aVector)
	{
		return Vector4<T>(aVector.x * aScalar, aVector.y * aScalar, aVector.z * aScalar, aVector.w * aScalar);
	}

	template <typename T>
	void operator*=(Vector4<T>& aVector, const T aScalar)
	{
		aVector = aVector * aScalar;
	}

#pragma endregion

#pragma region Division Operators

	template <typename T>
	Vector4<T> operator/(const Vector4<T>& aVector, const T aScalar)
	{
		if constexpr (std::is_floating_point<T>::value)
		{
			T dividerToMultiplier = T(1) / aScalar;
			return { aVector.x * dividerToMultiplier, aVector.y * dividerToMultiplier, aVector.z * dividerToMultiplier, aVector.w * dividerToMultiplier };
		}
		else
		{
			return { aVector.x / aScalar, aVector.y / aScalar, aVector.z / aScalar, aVector.w / aScalar };
		}
	}

	template <typename T>
	void operator/=(Vector4<T>& aVector, const T& aScalar)
	{
		aVector = aVector / aScalar;
	}

#pragma endregion

#pragma region Comparision Operators

	template <typename T>
	bool operator==(const Vector4<T>& lhs, const Vector4<T>& rhs)
	{
		if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w)
		{
			return true;
		}

		return false;
	}

	template <typename T>
	bool operator!=(const Vector4<T>& lhs, const Vector4<T>& rhs)
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
	std::ostream& operator<<(std::ostream& aOStream, const Vector4<T>& aVector)
	{
		aOStream << aVector.x << ", " << aVector.y << ", " << aVector.z << ", " << aVector.w;

		return aOStream;
	}

#pragma endregion


#pragma endregion

#pragma region Constructors

	template <typename T>
	inline Vector4<T>::Vector4()
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	template <typename T>
	inline Vector4<T>::Vector4(const T& aX, const T& aY, const T& aZ, const T& aW)
	{
		x = aX;
		y = aY;
		z = aZ;
		w = aW;
	}

#pragma endregion

	template <typename T>
	template <class TargetType>
	inline Vector4<TargetType> Vector4<T>::ToType() const
	{
		Vector4<TargetType> targetVector = 
		{
			static_cast<TargetType>(x), 
			static_cast<TargetType>(y),
			static_cast<TargetType>(z),
			static_cast<TargetType>(w)
		};

		return targetVector;
	}
	
	template <typename T>
	inline Tga::Vector4<T> Vector4<T>::ToTga() const
	{
		Tga::Vector4<T> tgaVector = 
		{
			static_cast<T>(x), 
			static_cast<T>(y),
		    static_cast<T>(z),
		    static_cast<T>(w)
		};

		return tgaVector;
	}

	template <typename T>
	inline T Vector4<T>::LengthSqr() const
	{
		return x * x + y * y + z * z + w * w;
	}

	template <typename T>
	inline T Vector4<T>::Length() const
	{
		return static_cast<T>(std::sqrt(LengthSqr()));
	}

	template <typename T>
	inline Vector4<T> Vector4<T>::GetNormalized() const
	{
		Vector4<T> normalizedVector = *this;
		normalizedVector.Normalize();

		return normalizedVector;
	}

	template <typename T>
	inline void Vector4<T>::Normalize()
	{
		T length = Length();

		if (length == T(0))
		{
			return;
		}

		x *= (1 / length);
		y *= (1 / length);
		z *= (1 / length);
		w *= (1 / length);
	}

	template <typename T>
	inline T Vector4<T>::Dot(const Vector4<T> &aVector) const
	{
		return (x * aVector.x) + (y * aVector.y) + (z * aVector.z) + (w * aVector.w);
	}

	template<typename T>
	inline T Vector4<T>::Dot(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		return (aVector0.x * aVector1.x) + (aVector0.y * aVector1.y) + (aVector0.z * aVector1.z) + (aVector0.w * aVector1.w);
	}

	template<typename T>
	inline Vector2<T> Vector4<T>::ToXY() const
	{
		return Vector2<T>(x, y);
	}

	template<typename T>
	inline Vector3<T> Vector4<T>::ToXYZ() const
	{
		return Vector3<T>(x, y, z);
	}
}

#pragma warning(pop)