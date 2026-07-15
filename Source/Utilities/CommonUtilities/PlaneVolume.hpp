#pragma once

#include <vector>

#include "Vector3.hpp"
#include "Plane.hpp"

namespace CommonUtilities
{
	template <typename T>
	class PlaneVolume
	{
		public:
			PlaneVolume();
			PlaneVolume(const std::vector<Plane<T>>& aPlaneList);

			void AddPlane(const Plane<T>& aPlane);

			bool IsInside(const Vector3<T>& aPosition) const;

			const std::vector<Plane<T>>& GetPlanes() const;

		private:
			std::vector<Plane<T>> myPlanes;
	};

	template<typename T>
	inline CommonUtilities::PlaneVolume<T>::PlaneVolume()
	{
	}

	template<typename T>
	inline CommonUtilities::PlaneVolume<T>::PlaneVolume(const std::vector<Plane<T>>& aPlaneList) 
		: myPlanes(aPlaneList)
	{
	}

	template<typename T>
	inline void CommonUtilities::PlaneVolume<T>::AddPlane(const Plane<T>& aPlane)
	{
		myPlanes.emplace_back(aPlane);
	}

	template<typename T>
	inline bool CommonUtilities::PlaneVolume<T>::IsInside(const Vector3<T>& aPosition) const
	{
		for (const Plane<T>& plane : myPlanes)
		{
			if (!plane.IsInside(aPosition))
			{
				return false;
			}
		}

		return true;
	}
	template<typename T>
	inline const std::vector<Plane<T>>& PlaneVolume<T>::GetPlanes() const
	{
		return myPlanes;
	}
}
