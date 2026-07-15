#pragma once

#include <iostream>
#include <cassert>
#include <initializer_list>

namespace CommonUtilities
{
	template <typename T>
	class Matrix4x4;

	template <typename T>
	class Vector3;
}

namespace CommonUtilities
{
	template <typename T>
	class Matrix3x3
	{
		public:
			static Matrix3x3<T> Identity;

			Matrix3x3<T>();
			Matrix3x3<T>(std::initializer_list<T> aMatrix);
			Matrix3x3<T>(const Matrix3x3<T>& aMatrix) = default;
			Matrix3x3<T>(const Matrix4x4<T>& aMatrix);

			Matrix3x3<T>& operator=(const Matrix3x3<T>& aMatrix) = default;

			T& operator()(const int aRow, const int aColumn);
			const T& operator()(const int aRow, const int aColumn) const;

			Matrix3x3<T> GetTranspose() const;

			Vector3<T> GetRow(const int aRow) const;
			void SetRow(const int aRow, const Vector3<T> aVector);

			Vector3<T> GetColumn(const int aColumn) const;
			void SetColumn(const int aColumn, const Vector3<T> aVector);

			static Matrix3x3<T> CreateRotationAroundX(const T aAngleInRadians);
			static Matrix3x3<T> CreateRotationAroundY(const T aAngleInRadians);
			static Matrix3x3<T> CreateRotationAroundZ(const T aAngleInRadians);

			T* GetData();
			const T* GetData() const;

		private:
			T myData[9];
	};

	using Matrix3x3f = Matrix3x3<float>;
	using Matrix3x3i = Matrix3x3<int>;
	using Matrix3x3u = Matrix3x3<unsigned int>;

	template<typename T>
	Matrix3x3<T> Matrix3x3<T>::Identity = Matrix3x3<T>();

#pragma region Operators

#pragma region Addition Operators

	template<typename T>
	Matrix3x3<T> operator+(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> result;

		const T* a = aMatrix0.GetData();
		const T* b = aMatrix1.GetData();

		T* r = result.GetData();

		r[0] = a[0] + b[0];
		r[1] = a[1] + b[1];
		r[2] = a[2] + b[2];
		r[3] = a[3] + b[3];
		r[4] = a[4] + b[4];
		r[5] = a[5] + b[5];
		r[6] = a[6] + b[6];
		r[7] = a[7] + b[7];
		r[8] = a[8] + b[8];

		return result;
	}

	template<typename T>
	void operator+=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		const T* a = aMatrix1.GetData();

		T* r = aMatrix0.GetData();

		r[0] += a[0];
		r[1] += a[1];
		r[2] += a[2];
		r[3] += a[3];
		r[4] += a[4];
		r[5] += a[5];
		r[6] += a[6];
		r[7] += a[7];
		r[8] += a[8];
	}

#pragma endregion

#pragma region Subtraction Operators

	template<typename T>
	Matrix3x3<T> operator-(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> result;

		const T* a = aMatrix0.GetData();
		const T* b = aMatrix1.GetData();

		T* r = result.GetData();

		r[0] = a[0] - b[0];
		r[1] = a[1] - b[1];
		r[2] = a[2] - b[2];
		r[3] = a[3] - b[3];
		r[4] = a[4] - b[4];
		r[5] = a[5] - b[5];
		r[6] = a[6] - b[6];
		r[7] = a[7] - b[7];
		r[8] = a[8] - b[8];

		return result;
	}

	template<typename T>
	void operator-=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		const T* a = aMatrix1.GetData();

		T* r = aMatrix0.GetData();

		r[0] -= a[0];
		r[1] -= a[1];
		r[2] -= a[2];
		r[3] -= a[3];
		r[4] -= a[4];
		r[5] -= a[5];
		r[6] -= a[6];
		r[7] -= a[7];
		r[8] -= a[8];
	}

#pragma endregion

#pragma region Multiplication Operators

	template<typename T>
	Matrix3x3<T> operator*(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> result;

		const T* a = aMatrix0.GetData();
		const T* b = aMatrix1.GetData();
		T* r = result.GetData();

		r[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
		r[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
		r[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];

		r[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
		r[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
		r[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];

		r[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
		r[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
		r[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];

		return result;
	}

	template<typename T>
	void operator*=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		aMatrix0 = aMatrix0 * aMatrix1;
	}

	template<typename T>
	Matrix3x3<T> operator*(const Matrix3x3<T>& aMatrix, const T& aScalar)
	{
		Matrix3x3<T> result;

		const T* a = aMatrix.GetData();

		T* r = result.GetData();

		r[0] = a[0] * aScalar;
		r[1] = a[1] * aScalar;
		r[2] = a[2] * aScalar;
		r[3] = a[3] * aScalar;
		r[4] = a[4] * aScalar;
		r[5] = a[5] * aScalar;
		r[6] = a[6] * aScalar;
		r[7] = a[7] * aScalar;
		r[8] = a[8] * aScalar;

		return result;
	}

	template<typename T>
	Matrix3x3<T> operator*(const T& aScalar, const Matrix3x3<T>& aMatrix)
	{
		return aMatrix * aScalar;
	}

	template<typename T>
	Vector3<T> operator*(const Vector3<T>& aVector, const Matrix3x3<T>& aMatrix)
	{
		const T* a = aMatrix.GetData();

		return Vector3<T>
		{
			aVector.x* a[0] + aVector.y * a[3] + aVector.z * a[6],
			aVector.x* a[1] + aVector.y * a[4] + aVector.z * a[7],
			aVector.x* a[2] + aVector.y * a[5] + aVector.z * a[8],
		};
	}

#pragma endregion

#pragma region Comparison Operators

	template<typename T>
	bool operator==(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
	{
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				if (lhs(row, column) != rhs(row, column))
				{
					return false;
				}
			}
		}

		return true;
	}

	template<typename T>
	bool operator!=(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
	{
		if (!(lhs == rhs))
		{
			return true;
		}

		return false;
	}

#pragma endregion

#pragma region Indexing Operators

	template<typename T>
	inline T& Matrix3x3<T>::operator()(const int aRow, const int aColumn)
	{
		assert(aRow >= 1 && aRow <= 3);
		assert(aColumn >= 1 && aColumn <= 3);

		return myData[(aRow - 1) * 3 + (aColumn - 1)];
	}

	template<typename T>
	inline const T& Matrix3x3<T>::operator()(const int aRow, const int aColumn) const
	{
		assert(aRow >= 1 && aRow <= 3);
		assert(aColumn >= 1 && aColumn <= 3);

		return myData[(aRow - 1) * 3 + (aColumn - 1)];
	}

#pragma endregion

#pragma region OStream Operator

	template<typename T>
	std::ostream& operator<<(std::ostream& aOStream, const Matrix3x3<T>& aMatrix)
	{
		for (int column = 1; column <= 3; column++)
		{
			aOStream << aMatrix.GetColumn(column) << std::endl;
		}

		return aOStream;
	}

#pragma endregion

#pragma endregion

#pragma region Constructors

	template<typename T>
	inline Matrix3x3<T>::Matrix3x3()
	{
		(*this)(1, 1) = 1;
		(*this)(2, 1) = 0;
		(*this)(3, 1) = 0;

		(*this)(1, 2) = 0;
		(*this)(2, 2) = 1;
		(*this)(3, 2) = 0;

		(*this)(1, 3) = 0;
		(*this)(2, 3) = 0;
		(*this)(3, 3) = 1;
	}

	template<typename T>
	inline Matrix3x3<T>::Matrix3x3(std::initializer_list<T> aMatrix)
	{
		assert(aMatrix.size() == 9);

		auto data = aMatrix.begin();

		(*this)(1, 1) = data[0];
		(*this)(1, 2) = data[1];
		(*this)(1, 3) = data[2];

		(*this)(2, 1) = data[3];
		(*this)(2, 2) = data[4];
		(*this)(2, 3) = data[5];

		(*this)(3, 1) = data[6];
		(*this)(3, 2) = data[7];
		(*this)(3, 3) = data[8];
	}

	template<typename T>
	inline Matrix3x3<T>::Matrix3x3(const Matrix4x4<T>& aMatrix)
	{
		(*this)(1, 1) = aMatrix(1, 1);
		(*this)(2, 1) = aMatrix(2, 1);
		(*this)(3, 1) = aMatrix(3, 1);

		(*this)(1, 2) = aMatrix(1, 2);
		(*this)(2, 2) = aMatrix(2, 2);
		(*this)(3, 2) = aMatrix(3, 2);

		(*this)(1, 3) = aMatrix(1, 3);
		(*this)(2, 3) = aMatrix(2, 3);
		(*this)(3, 3) = aMatrix(3, 3);
	}

#pragma endregion

	template<typename T>
	inline T* Matrix3x3<T>::GetData()
	{
		return myData;
	}

	template<typename T>
	inline const T* Matrix3x3<T>::GetData() const
	{
		return myData;
	}

	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::GetTranspose() const
	{
		Matrix3x3<T> t;

		const T* m = GetData();
		T* r = t.GetData();

		r[0] = m[0];  
		r[1] = m[3];  
		r[2] = m[6];
		r[3] = m[1];  
		r[4] = m[4];  
		r[5] = m[7];
		r[6] = m[2]; 
		r[7] = m[5];  
		r[8] = m[8];

		return t;
	}

	template<typename T>
	inline Vector3<T> Matrix3x3<T>::GetRow(const int aRow) const
	{
		assert(aRow >= 1 && aRow <= 3);

		return Vector3<T>((*this)(aRow, 1), (*this)(aRow, 2), (*this)(aRow, 3));
	}

	template<typename T>
	inline void Matrix3x3<T>::SetRow(const int aRow, const Vector3<T> aVector)
	{
		assert(aRow >= 1 && aRow <= 3);

		(*this)(aRow, 1) = aVector.x;
		(*this)(aRow, 2) = aVector.y;
		(*this)(aRow, 3) = aVector.z;
	}

	template<typename T>
	inline Vector3<T> Matrix3x3<T>::GetColumn(const int aColumn) const
	{
		assert(aColumn >= 1 && aColumn <= 3);

		return Vector3<T>((*this)(1, aColumn), (*this)(2, aColumn), (*this)(3, aColumn));
	}

	template<typename T>
	inline void Matrix3x3<T>::SetColumn(const int aColumn, const Vector3<T> aVector)
	{
		assert(aColumn >= 1 && aColumn <= 3);

		(*this)(1, aColumn) = aVector.x;
		(*this)(2, aColumn) = aVector.y;
		(*this)(3, aColumn) = aVector.z;
	}

	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundX(const T aAngleInRadians)
	{
		T s = std::sin(aAngleInRadians);
		T c = std::cos(aAngleInRadians);

		Matrix3x3<T> xRotationMatrix
		{
			1, 0, 0,
			0, c, s,
			0, -s, c
		};

		return xRotationMatrix;
	}

	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundY(const T aAngleInRadians)
	{
		T s = std::sin(aAngleInRadians);
		T c = std::cos(aAngleInRadians);

		Matrix3x3<T> yRotationMatrix
		{
			c, 0, -s,
			0, 1, 0,
			s, 0, c
		};

		return yRotationMatrix;
	}

	template<typename T>
	inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundZ(const T aAngleInRadians)
	{
		T s = std::sin(aAngleInRadians);
		T c = std::cos(aAngleInRadians);

		Matrix3x3<T> zRotationMatrix
		{
			c, s, 0,
			-s, c, 0,
			0, 0, 1
		};

		return zRotationMatrix;
	}
}