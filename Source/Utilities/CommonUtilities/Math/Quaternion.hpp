#pragma once

#include <cmath>
#include <algorithm>

#include "Vector3.hpp"
#include "Matrix3x3.hpp"
#include "MathUtils.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)

namespace CommonUtilities
{
	template <class T>
	class Quaternion
	{
		public:
			union
			{
				T values[4];

				struct
				{
					T w;
					T x;
					T y;
					T z;
				};
			};

			static Quaternion<T> Identity;

			Quaternion();
			Quaternion(const T& aW, const T& aX, const T& aY, const T& aZ);
			Quaternion(const T& aYaw, const T& aPitch, const T& aRoll);
			Quaternion(const CommonUtilities::Vector3<T>& aYawPitchRoll);
			Quaternion(const CommonUtilities::Vector3<T>& aVector, const T aAngle);
			Quaternion(const CommonUtilities::Matrix3x3<T>& aRotationMatrix);

			inline T Length() const;
			inline T Dot(const Quaternion<T>& aQuaternion) const;

			inline void Normalize();
			inline Quaternion<T> GetNormalized() const;
			inline Quaternion<T> GetConjugate() const;

			inline static CommonUtilities::Vector3<T> RotateVectorByQuaternion(const Quaternion<T>& aQuaternion, const CommonUtilities::Vector3<T>& aVector);

			inline CommonUtilities::Vector3<T> GetYawPitchRoll() const;

			inline static Quaternion<T> Lerp(const Quaternion<T>& aQuaternion0, const Quaternion<T>& aQuaternion1, const T aDelta);
			inline static Quaternion<T> Slerp(const Quaternion<T>& aQuaternion0, const Quaternion<T>& aQuaternion1, const T aDelta);
	};

	using Quaternionf = Quaternion<float>;
	using Quaternioni = Quaternion<int>;

	template<typename T>
	Quaternion<T> Quaternion<T>::Identity = Quaternion<T>(static_cast<T>(1), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0));

#pragma region Operators

#pragma region Negation Operator

	template <class T> 
	inline Quaternion<T> operator-(const Quaternion<T>& aQuaternion)
	{
		return Quaternion<T>
		(
			-aQuaternion.w, -aQuaternion.x, -aQuaternion.y, -aQuaternion.z
		);
	}

#pragma endregion

#pragma region Addition Operators

	template <class T> 
	inline Quaternion<T> operator-(const Quaternion<T>& aQuaternion0, const Quaternion<T>& aQuaternion1)
	{
		return Quaternion<T>
		(
			aQuaternion0.w - aQuaternion1.w, aQuaternion0.x - aQuaternion1.x, aQuaternion0.y - aQuaternion1.y, aQuaternion0.z - aQuaternion1.z
		);
	}

#pragma endregion

#pragma region Subtraction Operators

	template <class T> 
	inline Quaternion<T> operator+(const Quaternion<T>& aQuaternion0, const Quaternion<T>& aQuaternion1)
	{
		return Quaternion<T>
		(
			aQuaternion0.w + aQuaternion1.w, aQuaternion0.x + aQuaternion1.x, aQuaternion0.y + aQuaternion1.y, aQuaternion0.z + aQuaternion1.z
		);
	}

#pragma endregion

#pragma region Multiplication Operators

	template <class T>
	Quaternion<T> operator*(const Quaternion<T>& aQuaternion0, const Quaternion<T>& aQuaternion1)
	{
		return Quaternion<T>
		(
			aQuaternion0.w * aQuaternion1.w - aQuaternion0.x * aQuaternion1.x - aQuaternion0.y * aQuaternion1.y - aQuaternion0.z * aQuaternion1.z,
			aQuaternion0.w * aQuaternion1.x + aQuaternion0.x * aQuaternion1.w + aQuaternion0.y * aQuaternion1.z - aQuaternion0.z * aQuaternion1.y,
			aQuaternion0.w * aQuaternion1.y - aQuaternion0.x * aQuaternion1.z + aQuaternion0.y * aQuaternion1.w + aQuaternion0.z * aQuaternion1.x,
			aQuaternion0.w * aQuaternion1.z + aQuaternion0.x * aQuaternion1.y - aQuaternion0.y * aQuaternion1.x + aQuaternion0.z * aQuaternion1.w
		);
	}

	template <class T>
	Quaternion<T> operator*(const Quaternion<T>& aQuaternion, const T aScalar)
	{
		return Quaternion<T>
		(
			aQuaternion.w * aScalar, aQuaternion.x * aScalar, aQuaternion.y * aScalar, aQuaternion.z * aScalar
		);
	}

	template <class T>
	Quaternion<T> operator*(const T aScalar, const Quaternion<T>& aQuaternion)
	{
		return Quaternion<T>
			(
				aQuaternion.w * aScalar, aQuaternion.x * aScalar, aQuaternion.y * aScalar, aQuaternion.z * aScalar
			);
	}

	template <class T> 
	inline void operator*=(Quaternion<T>& aQuaternion, const T& aScalar)
	{
		aQuaternion.w *= aScalar;
		aQuaternion.x *= aScalar;
		aQuaternion.y *= aScalar;
		aQuaternion.z *= aScalar;
	}

#pragma endregion

#pragma endregion

#pragma region Constructors

	template<class T>
	inline Quaternion<T>::Quaternion()
	{
		w = static_cast<T>(1);
		x = static_cast<T>(0);
		y = static_cast<T>(0);
		z = static_cast<T>(0);
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const T& aW, const T& aX, const T& aY, const T& aZ)
	{
		w = aW;
		x = aX;
		y = aY;
		z = aZ;
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const T & aYaw, const T & aPitch, const T & aRoll)
	{
		T cy = cos(CommonUtilities::Math::DEG_TO_RAD<T> * aYaw * T(0.5));
		T sy = sin(CommonUtilities::Math::DEG_TO_RAD<T> * aYaw * T(0.5));
		T cr = cos(CommonUtilities::Math::DEG_TO_RAD<T> * aRoll * T(0.5));
		T sr = sin(CommonUtilities::Math::DEG_TO_RAD<T> * aRoll * T(0.5));
		T cp = cos(CommonUtilities::Math::DEG_TO_RAD<T> * aPitch * T(0.5));
		T sp = sin(CommonUtilities::Math::DEG_TO_RAD<T> * aPitch * T(0.5));

		w = cy * cr * cp + sy * sr * sp;
		x = cy * sr * cp - sy * cr * sp;
		y = cy * cr * sp + sy * sr * cp;
		z = sy * cr * cp - cy * sr * sp;
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const CommonUtilities::Vector3<T>& aYawPitchRoll)
	{
		T cx = cos(CommonUtilities::Math::DEG_TO_RAD<T> * aYawPitchRoll.x * T(0.5));
		T cy = cos(CommonUtilities::Math::DEG_TO_RAD<T> * aYawPitchRoll.y * T(0.5));
		T cz = cos(CommonUtilities::Math::DEG_TO_RAD<T> * aYawPitchRoll.z * T(0.5));
		T sx = sin(CommonUtilities::Math::DEG_TO_RAD<T> * aYawPitchRoll.x * T(0.5));
		T sy = sin(CommonUtilities::Math::DEG_TO_RAD<T> * aYawPitchRoll.y * T(0.5));
		T sz = sin(CommonUtilities::Math::DEG_TO_RAD<T> * aYawPitchRoll.z * T(0.5));

		w = cx * cy * cz + sx * sy * sz;
		x = sx * cy * cz - cx * sy * sz;
		y = cx * sy * cz + sx * cy * sz;
		z = cx * cy * sz - sx * sy * cz;
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const CommonUtilities::Vector3<T>& aVector, const T aAngle)
	{
		T halfAngle = CommonUtilities::Math::DEG_TO_RAD<T> * aAngle / T(2);
		T halfAngleSin = sin(halfAngle);

		w = cos(halfAngle);
		x = aVector.x * halfAngleSin;
		y = aVector.y * halfAngleSin;
		z = aVector.z * halfAngleSin;
	}

	template<class T>
	inline Quaternion<T>::Quaternion(const CommonUtilities::Matrix3x3<T>& aRotationMatrix)
	{
		T trace = aRotationMatrix(1, 1) + aRotationMatrix(2, 2) + aRotationMatrix(3, 3);

		if (trace > T(0))
		{
			T scalingFactor = std::sqrt(trace + T(1)) * T(2);
			w = T(0.25) * scalingFactor;
			x = (aRotationMatrix(2, 3) - aRotationMatrix(3, 2)) / scalingFactor;
			y = (aRotationMatrix(3, 1) - aRotationMatrix(1, 3)) / scalingFactor;
			z = (aRotationMatrix(1, 2) - aRotationMatrix(2, 1)) / scalingFactor;
		}
		else if (aRotationMatrix(1, 1) > aRotationMatrix(2, 2) && aRotationMatrix(1, 1) > aRotationMatrix(3, 3))
		{
			T scalingFactor = std::sqrt(T(1) + aRotationMatrix(1, 1) - aRotationMatrix(2, 2) - aRotationMatrix(3, 3)) * T(2);
			w = (aRotationMatrix(2, 3) - aRotationMatrix(3, 2)) / scalingFactor;
			x = T(0.25) * scalingFactor;
			y = (aRotationMatrix(2, 1) + aRotationMatrix(1, 2)) / scalingFactor;
			z = (aRotationMatrix(3, 1) + aRotationMatrix(1, 3)) / scalingFactor;
		}
		else if (aRotationMatrix(2, 2) > aRotationMatrix(3, 3))
		{
			T scalingFactor = std::sqrt(T(1) + aRotationMatrix(2, 2) - aRotationMatrix(1, 1) - aRotationMatrix(3, 3)) * T(2);
			w = (aRotationMatrix(3, 1) - aRotationMatrix(1, 3)) / scalingFactor;
			x = (aRotationMatrix(2, 1) + aRotationMatrix(1, 2)) / scalingFactor;
			y = T(0.25) * scalingFactor;
			z = (aRotationMatrix(3, 2) + aRotationMatrix(2, 3)) / scalingFactor;
		}
		else
		{
			T scalingFactor = std::sqrt(T(1) + aRotationMatrix(3, 3) - aRotationMatrix(1, 1) - aRotationMatrix(2, 2)) * T(2);
			w = (aRotationMatrix(1, 2) - aRotationMatrix(2, 1)) / scalingFactor;
			x = (aRotationMatrix(3, 1) + aRotationMatrix(1, 3)) / scalingFactor;
			y = (aRotationMatrix(3, 2) + aRotationMatrix(2, 3)) / scalingFactor;
			z = T(0.25) * scalingFactor;
		}

		Normalize();
	}

#pragma endregion

	template<class T>
	inline T Quaternion<T>::Length() const
	{
		return std::sqrt((w * w) + (x * x) + (y * y) + (z * z));
	}

	template<class T>
	inline T Quaternion<T>::Dot(const Quaternion<T>& aQuaternion) const
	{
		return w * aQuaternion.w + x * aQuaternion.x + y * aQuaternion.y + z * aQuaternion.z;
	}

	template<class T>
	inline void Quaternion<T>::Normalize()
	{
		T length = Length();

		if (length == T(0))
		{
			return;
		}

		T invLength = T(1) / length;

		x *= invLength;
		y *= invLength;
		z *= invLength;
		w *= invLength;
	}

	template<class T>
	inline Quaternion<T> Quaternion<T>::GetNormalized() const
	{
		Quaternion<T> normalizedQuaternion = *this;
		normalizedQuaternion.Normalize();

		return normalizedQuaternion;
	}

	template<class T>
	inline Quaternion<T> Quaternion<T>::GetConjugate() const
	{
		return Quaternion<T>(w, -x, -y, -z);
	}

	template<class T>
	inline CommonUtilities::Vector3<T> Quaternion<T>::RotateVectorByQuaternion(const Quaternion<T>& aQuaternion, const CommonUtilities::Vector3<T>& aVector)
	{
		const Vector3<T> b = { aQuaternion.x, aQuaternion.y, aQuaternion.z};
		const T b2 = b.x * b.x + b.y * b.y + b.z * b.z;

		return (aVector * (aQuaternion.w * aQuaternion.w - b2)) + (b * aVector.Dot(b) * T(2)) + (b.Cross(aVector) * (aQuaternion.w * T(2)));
	}

	template<class T>
	inline Vector3<T> Quaternion<T>::GetYawPitchRoll() const
	{
		float roll = std::atan2(T(2) * (w * x + y * z), T(1) - T(2) * (x * x + y * y));
		float yaw = std::asin(T(2) * (w * y - z * x));
		float pitch = std::atan2(T(2) * (w * z + x * y), T(1) - T(2) * (y * y + z * z));;

		return Vector3<T>(yaw, pitch, roll);
	}

	template<class T>
	inline Quaternion<T> Quaternion<T>::Lerp(const Quaternion<T>& aQuaternion0, const Quaternion<T>& aQuaternion1, const T aDelta)
	{
		Quaternion<T> result;

		Quaternion<T> b = aQuaternion1;

		if (aQuaternion0.Dot(b) < T(0))
		{
			b = -aQuaternion1;
		}

		result =
		{
			aQuaternion0.w + aDelta * (b.w - aQuaternion0.w),
			aQuaternion0.x + aDelta * (b.x - aQuaternion0.x),
			aQuaternion0.y + aDelta * (b.y - aQuaternion0.y),
			aQuaternion0.z + aDelta * (b.z - aQuaternion0.z)
		};

		return result.GetNormalized();
	}

	template<class T>
	inline Quaternion<T> Quaternion<T>::Slerp(const Quaternion<T>& aQuaternion0, const Quaternion<T>& aQuaternion1, const T aDelta)
	{
		Quaternion<T> b = aQuaternion1;

		T dot = std::clamp(aQuaternion0.Dot(b), T(-1), T(1));

		if (dot < T(0))
		{
			b = -aQuaternion1;
			dot = -dot;
		}

		if (dot > T(0.9995))
		{
			return (aQuaternion0 + (b - aQuaternion0) * aDelta).GetNormalized();
		}

		T angle = std::acos(dot);
		T sin_a = std::sin(angle);

		T s0 = std::sin((T(1) - aDelta) * angle) / sin_a;
		T s1 = std::sin(aDelta * angle) / sin_a;

		return (aQuaternion0 * s0) + (b * s1);
	}
}

#pragma warning(pop)