#pragma once

#include <random>
#include <type_traits>
#include <cassert>

namespace CommonUtilities
{
	namespace Random
	{
		template <typename T>
		inline T RandomValue(const T aMin, const T aMax)
		{
            static_assert(std::is_arithmetic_v<T>, "RandomValue requires an arithmetic type");

            assert(aMin < aMax);

            static thread_local std::mt19937 rng(std::random_device{}());

            if constexpr (std::is_integral_v<T>)
            {
                std::uniform_int_distribution<T> dist(aMin, aMax);
                return dist(rng);
            }
            else
            {
                std::uniform_real_distribution<T> dist(aMin, aMax);
                return dist(rng);
            }
		}
	}
}
