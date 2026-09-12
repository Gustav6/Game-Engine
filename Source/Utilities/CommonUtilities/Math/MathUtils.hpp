#pragma once

#include <vector>
#include <cassert>

namespace CommonUtilities
{
	namespace Math
	{
		template <typename T>
		constexpr T PI = static_cast<T>(3.14159265358979323846);

		template <typename T>
		constexpr T DEG_TO_RAD = static_cast<T>(PI<T> / T(180));

		template <typename T>
		constexpr T RAD_TO_DEG = static_cast<T>(T(180) / PI<T>);

		template <typename T>
		inline T Max(const T aFirst, const T aSecond)
		{
			return (aFirst < aSecond) ? aSecond : aFirst;
		}

		template <typename T>
		inline T Max(const std::vector<T>& aValues)
		{
			assert(!aValues.empty());

			T maxValue = aValues[0];

			for (size_t valueIndex = 1; valueIndex < aValues.size(); valueIndex++)
			{
				if (maxValue < aValues[valueIndex])
				{
					maxValue = aValues[valueIndex];
				}
			}

			return maxValue;
		}

		template <typename T>
		inline T Min(const T aFirst, const T aSecond)
		{
			return (aFirst < aSecond) ? aFirst : aSecond;
		}

		template <typename T>
		inline T Min(const std::vector<T> &aValues)
		{
			assert(!aValues.empty());

			T minValue = aValues[0];

			for (size_t valueIndex = 1; valueIndex < aValues.size(); valueIndex++)
			{
				if (minValue > aValues[valueIndex])
				{
					minValue = aValues[valueIndex];
				}
			}

			return minValue;
		}

		template <typename T>
		inline T Abs(const T aValue)
		{
			return (aValue < 0) ? -aValue : aValue;
		}

		template <typename T>
		inline T Clamp(const T aValue, const T aMin, const T aMax)
		{
			assert(aMin <= aMax);

			if (aValue < aMin)
			{
				return aMin;
			}
			if (aValue > aMax)
			{
				return aMax;
			}

			return aValue;
		}

		template <typename T>
		inline T Clamp01(const T aValue)
		{
			if (aValue < 0)
			{
				return 0;
			}
			if (aValue > 1)
			{
				return 1;
			}

			return aValue;
		}

		// TODO: Implement
		template <typename T>
		inline T Wrap(const T aValue, T aMin, T aMax)
		{
			

			return aValue;
		}

		template <typename T>
		inline T Lerp(const T aStart, const T aStop, const float aPercentage)
		{
			return aStart + aPercentage * (aStop - aStart);
		}

		template <typename T>
		inline void Swap(T &aFirst, T &aSecond)
		{
			T temp = aFirst;
			aFirst = aSecond;
			aSecond = temp;
		}
	}
}