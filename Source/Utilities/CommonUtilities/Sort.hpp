#pragma once

#include <vector>

namespace CommonUtilities
{
	namespace Algorithms
	{ 
		template<class T>
		void SelectionSort(std::vector<T>& aVectorOut)
		{
			if (aVectorOut.size() <= 1)
			{
				return;
			}

			for (size_t i = 0; i < aVectorOut.size(); i++)
			{
				size_t smallestValueIndex = i;

				for (size_t j = i + 1; j < aVectorOut.size(); j++)
				{
					if (aVectorOut[j] < aVectorOut[smallestValueIndex])
					{
						smallestValueIndex = j;
					}
				}

				if (smallestValueIndex != i)
				{
					T temp = aVectorOut[i];
					aVectorOut[i] = aVectorOut[smallestValueIndex];
					aVectorOut[smallestValueIndex] = temp;
				}
			}
		}

		template<class T>
		void BubbleSort(std::vector<T>& aVectorOut)
		{
			if (aVectorOut.size() <= 1)
			{
				return;
			}

			bool hasSwapped;

			for (size_t i = 0; i < aVectorOut.size(); i++)
			{
				hasSwapped = false;

				for (size_t j = 0; j < aVectorOut.size() - i - 1; j++)
				{
					if (aVectorOut[j + 1] < aVectorOut[j])
					{
						T temp = aVectorOut[j];
						aVectorOut[j] = aVectorOut[j + 1];
						aVectorOut[j + 1] = temp;

						hasSwapped = true;
					}
				}

				if (!hasSwapped)
				{
					break;
				}
			}
		}

		template<class T>
		void QuickSort(std::vector<T>& aVectorOut)
		{
			if (aVectorOut.size() <= 1)
			{
				return;
			}

			size_t pivotIndex = aVectorOut.size() / 2;

			T pivot = aVectorOut[pivotIndex];

			std::vector<T> leftVector;
			std::vector<T> rightVector;

			for (size_t index = 0; index < aVectorOut.size(); index++)
			{
				if (index == pivotIndex)
				{
					continue;
				}

				if (aVectorOut[index] < pivot)
				{
					leftVector.push_back(aVectorOut[index]);
				}
				else
				{
					rightVector.push_back(aVectorOut[index]);
				}
			}

			QuickSort(leftVector);
			QuickSort(rightVector);

			size_t indexOut = 0;

			for (size_t leftIndex = 0; leftIndex < leftVector.size(); leftIndex++)
			{
				aVectorOut[indexOut] = leftVector[leftIndex];
				indexOut++;
			}

			aVectorOut[indexOut] = pivot;
			indexOut++;

			for (size_t rightIndex = 0; rightIndex < rightVector.size(); rightIndex++)
			{
				aVectorOut[indexOut] = rightVector[rightIndex];
				indexOut++;
			}
		}

		template<class T>
		void MergeSort(std::vector<T>& aVectorOut)
		{
			if (aVectorOut.size() <= 1)
			{
				return;
			}

			size_t halfSize = aVectorOut.size() / 2;
			size_t leftSize = halfSize;
			size_t rightSize = aVectorOut.size() - halfSize;

			std::vector<T> leftVector(leftSize);
			std::vector<T> rightVector(rightSize);

			for (size_t leftIndex = 0; leftIndex < leftSize; leftIndex++)
			{
				leftVector[leftIndex] = aVectorOut[leftIndex];
			}

			for (size_t rightIndex = 0; rightIndex < rightSize; rightIndex++)
			{
				rightVector[rightIndex] = aVectorOut[halfSize + rightIndex];
			}

			MergeSort(leftVector);
			MergeSort(rightVector);

			size_t leftIndex = 0;
			size_t rightIndex = 0;
			size_t indexOut = 0;

			while (leftIndex < leftVector.size() && rightIndex < rightVector.size())
			{
				if (leftVector[leftIndex] < rightVector[rightIndex])
				{
					aVectorOut[indexOut] = leftVector[leftIndex];

					leftIndex++;
				}
				else
				{
					aVectorOut[indexOut] = rightVector[rightIndex];

					rightIndex++;
				}

				indexOut++;
			}

			while (leftIndex < leftVector.size())
			{
				aVectorOut[indexOut] = leftVector[leftIndex];

				indexOut++;
				leftIndex++;
			}

			while (rightIndex < rightVector.size())
			{
				aVectorOut[indexOut] = rightVector[rightIndex];

				indexOut++;
				rightIndex++;
			}
		}

		template <typename T>
		bool IsListSorted(const std::vector<T>& aVector)
		{
			for (size_t index = 1; index < aVector.size(); index++)
			{
				if (aVector[index] < aVector[index - 1])
				{
					return false;
				}
			}

			return true;
		}
	}
}