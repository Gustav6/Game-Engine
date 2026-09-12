#pragma once

#include <iostream>
#include <cassert>
#include <cmath>
#include <initializer_list>

namespace CommonUtilities
{
	template <typename T>
	class Matrix3x3;

	template <typename T>
	class Vector4;

	template <typename T>
	class Vector3;
}

namespace CommonUtilities
{
	template <typename T>
	class Matrix4x4
	{
		public:
			static Matrix4x4<T> Identity;

			Matrix4x4<T>();
			Matrix4x4<T>(std::initializer_list<T> aMatrix);
			Matrix4x4<T>(const Matrix3x3<T>& aMatrix);
			Matrix4x4<T>(const Matrix4x4<T>& aMatrix) = default;

			Matrix4x4<T>& operator=(const Matrix4x4<T>& aMatrix) = default;

			T& operator()(const int aRow, const int aColumn);
			const T& operator()(const int aRow, const int aColumn) const;

			Matrix4x4<T> GetTranspose() const;

			Vector4<T> GetRow(const int aRow) const;
			void SetRow(const int aRow, const Vector4<T> aVector);

			Vector4<T> GetColumn(const int aColumn) const;
			void SetColumn(const int aColumn, const Vector4<T> aVector);

			static Matrix4x4<T> CreateRotationAroundX(const T aAngleInRadians);
			static Matrix4x4<T> CreateRotationAroundY(const T aAngleInRadians);
			static Matrix4x4<T> CreateRotationAroundZ(const T aAngleInRadians);

			Vector3<T> GetForward() const;
			Vector3<T> GetUp() const;
			Vector3<T> GetRight() const;
			Vector3<T> GetTranslation() const;

			Matrix4x4<T> GetInverse() const;
			Matrix4x4<T> GetFastInverse() const;
			Matrix4x4<T> GetGeneralInverse() const;

			void SetTranslation(const Vector3<T>& aTranslation);
			void SetScaling(const Vector3<T>& aScaling);

			T* GetData();
			const T* GetData() const;

			static Matrix4x4<T> CreateOrthographicProjection(float aNearPlane, float aFarPlane, float aWidth, float aHeight);
			static Matrix4x4<T> CreateOrthographicProjection(float aNearPlane, float aFarPlane, float aLeft, float aRight, float aBottom, float aTop);
			static Matrix4x4<T> CreatePerspectiveProjection(float aHorizontalFov, float aNearPlane, float aFarPlane, float aAspectRatio);
			static Matrix4x4<T> CreateLookAt(const Vector3<T>& aPosition, const Vector3<T>& aDirection, const Vector3<T>& aUp);

			static constexpr Matrix4x4<T> CreateNDCToTextureSpace();

		private:
			T myData[16];
	};

	using Matrix4x4f = Matrix4x4<float>;
	using Matrix4x4i = Matrix4x4<int>;
	using Matrix4x4u = Matrix4x4<unsigned int>;
		
	template<typename T>
	Matrix4x4<T> Matrix4x4<T>::Identity = Matrix4x4<T>();

#pragma region Operators

#pragma region Addition Operators

	template <typename T>
	Matrix4x4<T> operator+(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1) 
	{
		Matrix4x4<T> result;

		const T* a = aMatrix0.GetData();
		const T* b = aMatrix1.GetData();

		T* r = result.GetData();

		r[0]  = a[0]  + b[0];
		r[1]  = a[1]  + b[1];
		r[2]  = a[2]  + b[2];
		r[3]  = a[3]  + b[3];
		r[4]  = a[4]  + b[4];
		r[5]  = a[5]  + b[5];
		r[6]  = a[6]  + b[6];
		r[7]  = a[7]  + b[7];
		r[8]  = a[8]  + b[8];
		r[9]  = a[9]  + b[9];
		r[10] = a[10] + b[10];
		r[11] = a[11] + b[11];
		r[12] = a[12] + b[12];
		r[13] = a[13] + b[13];
		r[14] = a[14] + b[14];
		r[15] = a[15] + b[15];

		return result;
	}

	template <typename T>
	void operator+=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		const T* a = aMatrix1.GetData();

		T* r = aMatrix0.GetData();

		r[0]  += a[0];
		r[1]  += a[1];
		r[2]  += a[2];
		r[3]  += a[3];
		r[4]  += a[4];
		r[5]  += a[5];
		r[6]  += a[6];
		r[7]  += a[7];
		r[8]  += a[8];
		r[9]  += a[9];
		r[10] += a[10];
		r[11] += a[11];
		r[12] += a[12];
		r[13] += a[13];
		r[14] += a[14];
		r[15] += a[15];
	}

#pragma endregion

#pragma region Subtraction Operators

	template <typename T>
	Matrix4x4<T> operator-(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1) 
	{
		Matrix4x4<T> result;

		const T* a = aMatrix0.GetData();
		const T* b = aMatrix1.GetData();

		T* r = result.GetData();

		r[0]  = a[0]  - b[0];
		r[1]  = a[1]  - b[1];
		r[2]  = a[2]  - b[2];
		r[3]  = a[3]  - b[3];
		r[4]  = a[4]  - b[4];
		r[5]  = a[5]  - b[5];
		r[6]  = a[6]  - b[6];
		r[7]  = a[7]  - b[7];
		r[8]  = a[8]  - b[8];
		r[9]  = a[9]  - b[9];
		r[10] = a[10] - b[10];
		r[11] = a[11] - b[11];
		r[12] = a[12] - b[12];
		r[13] = a[13] - b[13];
		r[14] = a[14] - b[14];
		r[15] = a[15] - b[15];

		return result;
	}

	template <typename T>
	void operator-=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1) 
	{
		const T* a = aMatrix1.GetData();

		T* r = aMatrix0.GetData();

		r[0]  -= a[0];
		r[1]  -= a[1];
		r[2]  -= a[2];
		r[3]  -= a[3];
		r[4]  -= a[4];
		r[5]  -= a[5];
		r[6]  -= a[6];
		r[7]  -= a[7];
		r[8]  -= a[8];
		r[9]  -= a[9];
		r[10] -= a[10];
		r[11] -= a[11];
		r[12] -= a[12];
		r[13] -= a[13];
		r[14] -= a[14];
		r[15] -= a[15];
	}

#pragma endregion

#pragma region Multiplication Operators

	template <typename T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1) 
	{
		Matrix4x4<T> result;

		const T* a = aMatrix0.GetData();
		const T* b = aMatrix1.GetData();
		T* r = result.GetData();

		r[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8]  + a[3] * b[12];
		r[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9]  + a[3] * b[13];
		r[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10] + a[3] * b[14];
		r[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3] * b[15];

		r[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8]  + a[7] * b[12];
		r[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9]  + a[7] * b[13];
		r[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10] + a[7] * b[14];
		r[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7] * b[15];

		r[8]  = a[8] * b[0] + a[9] * b[4] + a[10] * b[8]  + a[11] * b[12];
		r[9]  = a[8] * b[1] + a[9] * b[5] + a[10] * b[9]  + a[11] * b[13];
		r[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10] + a[11] * b[14];
		r[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11] * b[15];

		r[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8]  + a[15] * b[12];
		r[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9]  + a[15] * b[13];
		r[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + a[15] * b[14];
		r[15] = a[12] * b[3] + a[13] * b[7] + a[14] * b[11] + a[15] * b[15];

		return result;
	}

	template <typename T>
	void operator*=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1) 
	{
		aMatrix0 = aMatrix0 * aMatrix1;
	}

	template <typename T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& aMatrix, const T& aScalar)
	{
		Matrix4x4<T> result;

		const T* a = aMatrix.GetData();

		T* r = result.GetData();

		r[0]  = a[0]  * aScalar;
		r[1]  = a[1]  * aScalar;
		r[2]  = a[2]  * aScalar;
		r[3]  = a[3]  * aScalar;
		r[4]  = a[4]  * aScalar;
		r[5]  = a[5]  * aScalar;
		r[6]  = a[6]  * aScalar;
		r[7]  = a[7]  * aScalar;
		r[8]  = a[8]  * aScalar;
		r[9]  = a[9]  * aScalar;
		r[10] = a[10] * aScalar;
		r[11] = a[11] * aScalar;
		r[12] = a[12] * aScalar;
		r[13] = a[13] * aScalar;
		r[14] = a[14] * aScalar;
		r[15] = a[15] * aScalar;

		return result;
	}

	template <typename T>
	Matrix4x4<T> operator*(const T& aScalar, const Matrix4x4<T>& aMatrix)
	{
		return aMatrix * aScalar;
	}

	template <typename T>
	Vector4<T> operator*(const Vector4<T>& aVector, const Matrix4x4<T>& aMatrix)
	{
		const T* a = aMatrix.GetData();

		return Vector4<T>
		{
			aVector.x* a[0] + aVector.y * a[4] + aVector.z * a[8]  + aVector.w * a[12],
			aVector.x* a[1] + aVector.y * a[5] + aVector.z * a[9]  + aVector.w * a[13],
			aVector.x* a[2] + aVector.y * a[6] + aVector.z * a[10] + aVector.w * a[14],
			aVector.x* a[3] + aVector.y * a[7] + aVector.z * a[11] + aVector.w * a[15],
		};
	}

	template <typename T>
	void operator*=(Vector4<T>& aVector, const Matrix4x4<T>& aMatrix)
	{
		aVector = aVector * aMatrix;
	}

#pragma endregion

#pragma region Comparision Operators

	template <typename T>
	bool operator==(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs) 
	{
		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				if (lhs(row, column) != rhs(row, column))
				{
					return false;
				}
			}
		}

		return true;
	}

	template <typename T>
	bool operator!=(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs)
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
	inline T& Matrix4x4<T>::operator()(const int aRow, const int aColumn)
	{
		assert(aRow >= 1 && aRow <= 4);
		assert(aColumn >= 1 && aColumn <= 4);

		return myData[(aRow - 1) * 4 + (aColumn - 1)];
	}

	template<typename T>
	inline const T& Matrix4x4<T>::operator()(const int aRow, const int aColumn) const
	{
		assert(aRow >= 1 && aRow <= 4);
		assert(aColumn >= 1 && aColumn <= 4);

		return myData[(aRow - 1) * 4 + (aColumn - 1)];
	}

#pragma endregion

#pragma region OStream Operator

	template<typename T>
	std::ostream& operator<<(std::ostream& aOStream, const Matrix4x4<T>& aMatrix)
	{
		for (int column = 1; column <= 4; column++)
		{
			aOStream << aMatrix.GetColumn(column) << std::endl;
		}

		return aOStream;
	}

#pragma endregion

#pragma endregion

#pragma region Constructors

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4()
	{
		(*this)(1, 1) = 1;
		(*this)(2, 1) = 0;
		(*this)(3, 1) = 0;
		(*this)(4, 1) = 0;

		(*this)(1, 2) = 0;
		(*this)(2, 2) = 1;
		(*this)(3, 2) = 0;
		(*this)(4, 2) = 0;

		(*this)(1, 3) = 0;
		(*this)(2, 3) = 0;
		(*this)(3, 3) = 1;
		(*this)(4, 3) = 0;

		(*this)(1, 4) = 0;
		(*this)(2, 4) = 0;
		(*this)(3, 4) = 0;
		(*this)(4, 4) = 1;
	}

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4(std::initializer_list<T> aMatrix)
	{
		assert(aMatrix.size() == 16);

		auto data = aMatrix.begin();

		(*this)(1, 1) = data[0];
		(*this)(1, 2) = data[1];
		(*this)(1, 3) = data[2];
		(*this)(1, 4) = data[3];

		(*this)(2, 1) = data[4];
		(*this)(2, 2) = data[5];
		(*this)(2, 3) = data[6];
		(*this)(2, 4) = data[7];

		(*this)(3, 1) = data[8];
		(*this)(3, 2) = data[9];
		(*this)(3, 3) = data[10];
		(*this)(3, 4) = data[11];

		(*this)(4, 1) = data[12];
		(*this)(4, 2) = data[13];
		(*this)(4, 3) = data[14];
		(*this)(4, 4) = data[15];
	}

	template<typename T>
	inline Matrix4x4<T>::Matrix4x4(const Matrix3x3<T>& aMatrix)
	{
		(*this)(1, 1) = aMatrix(1, 1);
		(*this)(2, 1) = aMatrix(2, 1);
		(*this)(3, 1) = aMatrix(3, 1);
		(*this)(4, 1) = 0;

		(*this)(1, 2) = aMatrix(1, 2);
		(*this)(2, 2) = aMatrix(2, 2);
		(*this)(3, 2) = aMatrix(3, 2);
		(*this)(4, 2) = 0;

		(*this)(1, 3) = aMatrix(1, 3);
		(*this)(2, 3) = aMatrix(2, 3);
		(*this)(3, 3) = aMatrix(3, 3);
		(*this)(4, 3) = 0;

		(*this)(1, 4) = 0;
		(*this)(2, 4) = 0;
		(*this)(3, 4) = 0;
		(*this)(4, 4) = 1;
	}

#pragma endregion

	template<typename T>
	inline T* Matrix4x4<T>::GetData()
	{
		return myData;
	}

	template<typename T>
	inline const T* Matrix4x4<T>::GetData() const
	{
		return myData;
	}

	template<typename T>
	inline Vector3<T> Matrix4x4<T>::GetTranslation() const
	{
		return Vector3<T>(myData[12], myData[13], myData[14]);
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::GetTranspose() const
	{
		Matrix4x4<T> t;

		const T* m = GetData();
		T* r = t.GetData();

		r[0] = m[0];   
		r[1] = m[4];  
		r[2] = m[8];   
		r[3] = m[12];
		r[4] = m[1];   
		r[5] = m[5];   
		r[6] = m[9];    
		r[7] = m[13];
		r[8] = m[2];   
		r[9] = m[6];   
		r[10] = m[10];  
		r[11] = m[14];
		r[12] = m[3];  
		r[13] = m[7];  
		r[14] = m[11];  
		r[15] = m[15];

		return t;
	}

	template<typename T>
	inline Vector4<T> Matrix4x4<T>::GetRow(const int aRow) const
	{
		assert(aRow >= 1 && aRow <= 4);

		return Vector4<T>((*this)(aRow, 1), (*this)(aRow, 2), (*this)(aRow, 3), (*this)(aRow, 4));
	}

	template<typename T>
	inline void Matrix4x4<T>::SetRow(const int aRow, const Vector4<T> aVector)
	{
		assert(aRow >= 1 && aRow <= 4);

		(*this)(aRow, 1) = aVector.x;
		(*this)(aRow, 2) = aVector.y;
		(*this)(aRow, 3) = aVector.z;
		(*this)(aRow, 4) = aVector.w;
	}

	template<typename T>
	inline Vector4<T> Matrix4x4<T>::GetColumn(const int aColumn) const
	{
		assert(aColumn >= 1 && aColumn <= 4);

		return Vector4<T>((*this)(1, aColumn), (*this)(2, aColumn), (*this)(3, aColumn), (*this)(4, aColumn));
	}

	template<typename T>
	inline void Matrix4x4<T>::SetColumn(const int aColumn, const Vector4<T> aVector)
	{
		assert(aColumn >= 1 && aColumn <= 4);

		(*this)(1, aColumn) = aVector.x;
		(*this)(2, aColumn) = aVector.y;
		(*this)(3, aColumn) = aVector.z;
		(*this)(4, aColumn) = aVector.w;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundX(const T aAngleInRadians)
	{
		T s = std::sin(aAngleInRadians);
		T c = std::cos(aAngleInRadians);

		Matrix4x4<T> xRotationMatrix
		{
			1, 0, 0, 0,
			0, c, s, 0,
			0, -s, c, 0,
			0, 0, 0, 1
		};

		return xRotationMatrix;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundY(const T aAngleInRadians)
	{
		T s = std::sin(aAngleInRadians);
		T c = std::cos(aAngleInRadians);

		Matrix4x4<T> yRotationMatrix
		{
			c, 0, -s, 0,
			0, 1, 0, 0,
			s, 0, c, 0,
			0, 0, 0, 1
		};

		return yRotationMatrix;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundZ(const T aAngleInRadians)
	{
		T s = std::sin(aAngleInRadians);
		T c = std::cos(aAngleInRadians);

		Matrix4x4<T> zRotationMatrix
		{
			c, s, 0, 0,
			-s, c, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};

		return zRotationMatrix;
	}

	template<typename T>
	inline Vector3<T> Matrix4x4<T>::GetForward() const
	{
		return { (*this)(3,1), (*this)(3,2), (*this)(3,3) };
	}

	template<typename T>
	inline Vector3<T> Matrix4x4<T>::GetUp() const
	{
		return { (*this)(2,1), (*this)(2,2), (*this)(2,3) };
	}

	template<typename T>
	inline Vector3<T> Matrix4x4<T>::GetRight() const
	{
		return { (*this)(1,1), (*this)(1,2), (*this)(1,3) };
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::GetInverse() const
	{
		T xScale = (*this)(1, 1) * (*this)(1, 1) + (*this)(2, 1) * (*this)(2, 1) + (*this)(3, 1) * (*this)(3, 1);
		T yScale = (*this)(1, 2) * (*this)(1, 2) + (*this)(2, 2) * (*this)(2, 2) + (*this)(3, 2) * (*this)(3, 2);
		T zScale = (*this)(1, 3) * (*this)(1, 3) + (*this)(2, 3) * (*this)(2, 3) + (*this)(3, 3) * (*this)(3, 3);

		Matrix4x4<T> result;

		result(1, 1) = (*this)(1, 1) / xScale;
		result(1, 2) = (*this)(2, 1) / yScale;
		result(1, 3) = (*this)(3, 1) / zScale;

		result(2, 1) = (*this)(1, 2) / xScale;
		result(2, 2) = (*this)(2, 2) / yScale;
		result(2, 3) = (*this)(3, 2) / zScale;

		result(3, 1) = (*this)(1, 3) / xScale;
		result(3, 2) = (*this)(2, 3) / yScale;
		result(3, 3) = (*this)(3, 3) / zScale;

		result(1, 4) = 0;
		result(2, 4) = 0;
		result(3, 4) = 0;
		result(4, 4) = 1;

		T x = -(*this)(4, 1);
		T y = -(*this)(4, 2);
		T z = -(*this)(4, 3);

		result(4, 1) = x * result(1, 1) + y * result(2, 1) + z * result(3, 1);
		result(4, 2) = x * result(1, 2) + y * result(2, 2) + z * result(3, 2);
		result(4, 3) = x * result(1, 3) + y * result(2, 3) + z * result(3, 3);

		return result;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::GetFastInverse() const
	{
		Matrix4x4<T> result;

		result(1, 1) = (*this)(1, 1); 
		result(1, 2) = (*this)(2, 1); 
		result(1, 3) = (*this)(3, 1);

		result(2, 1) = (*this)(1, 2); 
		result(2, 2) = (*this)(2, 2); 
		result(2, 3) = (*this)(3, 2);

		result(3, 1) = (*this)(1, 3); 
		result(3, 2) = (*this)(2, 3); 
		result(3, 3) = (*this)(3, 3);

		result(1, 4) = 0; 
		result(2, 4) = 0;
		result(3, 4) = 0;
		result(4, 4) = 1;

		T x = -(*this)(4, 1);
		T y = -(*this)(4, 2);
		T z = -(*this)(4, 3);

		result(4, 1) = x * result(1, 1) + y * result(2, 1) + z * result(3, 1);
		result(4, 2) = x * result(1, 2) + y * result(2, 2) + z * result(3, 2);
		result(4, 3) = x * result(1, 3) + y * result(2, 3) + z * result(3, 3);

		return result;
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::GetGeneralInverse() const
	{
		const T* m = GetData();
		T inv[16] = {};

		inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
		inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
		inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
		inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

		inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
		inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
		inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
		inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

		inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
		inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
		inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
		inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

		inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
		inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
		inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
		inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

		T det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

		assert(det != static_cast<T>(0));

		T invDet = static_cast<T>(1) / det;

		Matrix4x4<T> result;
		T* r = result.GetData();

		for (int i = 0; i < 16; ++i)
		{
			r[i] = inv[i] * invDet;
		}

		return result;
	}

	template<typename T>
	inline void Matrix4x4<T>::SetTranslation(const Vector3<T>& aTranslation)
	{
		myData[12] = aTranslation.x; 
		myData[13] = aTranslation.y;
		myData[14] = aTranslation.z;
	}

	template<typename T>
	inline void Matrix4x4<T>::SetScaling(const Vector3<T>& aScaling)
	{
		myData[0] = aScaling.x;
		myData[5] = aScaling.y;
		myData[10] = aScaling.z;
	}

	template<typename T>
	Matrix4x4<T> Matrix4x4<T>::CreateOrthographicProjection(float aNearPlane, float aFarPlane, float aWidth, float aHeight)
	{
		assert(aNearPlane > 0.0f && aFarPlane > aNearPlane);

		float scaleX = 2.0f / static_cast<float>(aWidth);
		float scaleY = 2.0f / static_cast<float>(aHeight);

		float depthNormalize = 1.0f / (aFarPlane - aNearPlane);
		float depthNearOffset = aNearPlane / (aNearPlane - aFarPlane);

		return Matrix4x4<T>
		{
			scaleX, 0, 		0, 				 0,
			0, 		scaleY, 0, 				 0,
			0, 		0, 		depthNormalize,  0,
			0, 		0, 		depthNearOffset, 1
		};
	}

	template<typename T>
	Matrix4x4<T> Matrix4x4<T>::CreateOrthographicProjection(float aNearPlane, float aFarPlane, float aLeft, float aRight, float aBottom, float aTop)
	{
		assert(aNearPlane < aFarPlane);

		float scaleX = 2.0f / static_cast<float>(aRight - aLeft);
		float scaleY = 2.0f / static_cast<float>(aTop - aBottom);
		
		const float offsetX = -(aRight + aLeft) / (aRight - aLeft);
		const float offsetY = -(aTop + aBottom) / (aTop - aBottom);

		float depthNormalize = 1.0f / (aFarPlane - aNearPlane);
		float depthNearOffset = aNearPlane / (aNearPlane - aFarPlane);

		return Matrix4x4<T>
		{
			scaleX,  0, 	  0, 			   0,
			0, 		 scaleY,  0, 			   0,
			0, 		 0, 	  depthNormalize,  0,
			offsetX, offsetY, depthNearOffset, 1
		};
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreatePerspectiveProjection(float aHorizontalFov, float aNearPlane, float aFarPlane, float aAspectRatio)
	{
		assert(aNearPlane > 0.0f && aFarPlane > aNearPlane);

		float inverseAspectRatio = 1.0f / aAspectRatio;

		float hFovRad = aHorizontalFov * 3.14159265358979323846f / 180.0f;
		float vFovRad = 2.0f * std::atan(std::tan(hFovRad * 0.5f) * inverseAspectRatio);

		float zoomX = 1.0f / std::tan(hFovRad * 0.5f);
		float zoomY = 1.0f / std::tan(vFovRad * 0.5f);

		float depthNormalize = aFarPlane / (aFarPlane - aNearPlane);
		float depthNearOffset = -(aNearPlane * aFarPlane) / (aFarPlane - aNearPlane);;

		return Matrix4x4<T>
		{
			zoomX,	0	 , 0			  ,	0,
			0	 ,	zoomY, 0			  ,	0,
			0	 ,	0	 , depthNormalize ,	1,
			0	 ,	0	 , depthNearOffset, 0
		};
	}

	template<typename T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateLookAt(const Vector3<T>& aPosition, const Vector3<T>& aDirection, const Vector3<T>& aUp)
	{
		Vector3<T> zAxis = aDirection.GetNormalized();
		Vector3<T> xAxis = aUp.Cross(zAxis).GetNormalized();
		Vector3<T> yAxis = zAxis.Cross(xAxis);

		T x = Vector3<T>::Dot(xAxis, aPosition);
		T y = Vector3<T>::Dot(yAxis, aPosition);
		T z = Vector3<T>::Dot(zAxis, aPosition);

		return Matrix4x4<T>
		{
			xAxis.x, xAxis.y, xAxis.z, 0,
			yAxis.x, yAxis.y, yAxis.z, 0,
			zAxis.x, zAxis.y, zAxis.z, 0,
			-x	   , -y		, -z	 , 1
		};
	}

	template<typename T>
	inline constexpr Matrix4x4<T> Matrix4x4<T>::CreateNDCToTextureSpace()
	{
		return Matrix4x4<T>
		{
			static_cast<T>(0.5), static_cast<T>(0)	 , static_cast<T>(0), static_cast<T>(0),
			static_cast<T>(0)  , static_cast<T>(-0.5), static_cast<T>(0), static_cast<T>(0),
			static_cast<T>(0)  , static_cast<T>(0)	 , static_cast<T>(1), static_cast<T>(0),
			static_cast<T>(0.5), static_cast<T>(0.5f), static_cast<T>(0), static_cast<T>(1)
		};
	}
}