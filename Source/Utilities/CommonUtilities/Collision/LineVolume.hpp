#pragma once

#include <vector>

#include "Vector2.hpp"
#include "Line.hpp"

namespace CommonUtilities
{
	template <typename T>
	class LineVolume
	{
		public:
			LineVolume();
			LineVolume(const std::vector<Line<T>>& aLineList);

			void AddLine(const Line<T>& aLine);

			bool IsInside(const Vector2<T>& aPosition) const;

		private:
			std::vector<Line<T>> myLines;
	};

	template<typename T>
	inline CommonUtilities::LineVolume<T>::LineVolume()
	{
	}

	template<typename T>
	inline CommonUtilities::LineVolume<T>::LineVolume(const std::vector<Line<T>>& aLineList) : myLines(aLineList)
	{
	}

	template<typename T>
	inline void CommonUtilities::LineVolume<T>::AddLine(const Line<T>& aLine)
	{
		myLines.emplace_back(aLine);
	}

	template<typename T>
	inline bool CommonUtilities::LineVolume<T>::IsInside(const Vector2<T>& aPosition) const
	{
		for (const Line<T>& line : myLines)
		{
			if (!line.IsInside(aPosition))
			{
				return false;
			}
		}

		return true;
	}
}