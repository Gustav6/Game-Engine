#pragma once

#include <cassert>

namespace CommonUtilities
{
	enum class HeapType
	{
		Min,
		Max
	};

	template <class T, HeapType heapType>
	class Heap
	{
		public:
			Heap();
			~Heap();

			int GetSize() const;

			const T& GetTop() const;

			void Enqueue(const T& aElement);
			T Dequeue();

		private:
			unsigned int mySize;
			unsigned int myCapacity;

			T* myData;
	};

	template<class T, HeapType heapType>
	inline Heap<T, heapType>::Heap()
	{
		mySize = 0;
		myCapacity = 5;

		myData = new T[myCapacity];
	}

	template<class T, HeapType heapType>
	inline Heap<T, heapType>::~Heap()
	{
		delete[] myData;
	}

	template<class T, HeapType heapType>
	inline int Heap<T, heapType>::GetSize() const
	{
		return mySize;
	}

	template<class T, HeapType heapType>
	inline const T& Heap<T, heapType>::GetTop() const
	{
		assert(mySize > 0);

		return myData[0];
	}

	template<class T, HeapType heapType>
	inline void Heap<T, heapType>::Enqueue(const T& aElement)
	{
		if (mySize >= myCapacity)
		{
			T* resizedArray = new T[myCapacity + 5];

			for (int i = 0; i < static_cast<int>(mySize); i++)
			{
				resizedArray[i] = myData[i];
			}

			delete[] myData;

			myData = resizedArray;
			myCapacity += 5;
		}

		myData[mySize] = aElement;

		int currentIndex = mySize;
		int parentIndex = (currentIndex - 1) / 2;

		if (heapType == HeapType::Max)
		{
			while (currentIndex > 0 && myData[parentIndex] < myData[currentIndex])
			{
				std::swap(myData[parentIndex], myData[currentIndex]);

				currentIndex = parentIndex;
				parentIndex = (currentIndex - 1) / 2;
			}
		}
		else 
		{
			while (currentIndex > 0 && myData[currentIndex] < myData[parentIndex])
			{
				std::swap(myData[parentIndex], myData[currentIndex]);

				currentIndex = parentIndex;
				parentIndex = (currentIndex - 1) / 2;
			}
		}

		mySize++;
	}

	template<class T, HeapType heapType>
	inline T Heap<T, heapType>::Dequeue()
	{
		assert(mySize > 0);

		T myLargestElement = myData[0];

		mySize--;

		myData[0] = myData[mySize];

		int currentIndex = 0;
		
		while (currentIndex < static_cast<int>(mySize))
		{
			int leftChildIndex = 2 * currentIndex + 1;
			int rightChildIndex = 2 * currentIndex + 2;

			if (heapType == HeapType::Max)
			{
				if (rightChildIndex < static_cast<int>(mySize) && myData[currentIndex] < myData[rightChildIndex] && myData[leftChildIndex] < myData[rightChildIndex])
				{
					std::swap(myData[rightChildIndex], myData[currentIndex]);

					currentIndex = rightChildIndex;
				}
				else if (leftChildIndex < static_cast<int>(mySize) && myData[currentIndex] < myData[leftChildIndex])
				{
					std::swap(myData[leftChildIndex], myData[currentIndex]);

					currentIndex = leftChildIndex;
				}
				else
				{
					break;
				}
			}
			else 
			{
				if (rightChildIndex < static_cast<int>(mySize) && myData[rightChildIndex] < myData[currentIndex] && myData[rightChildIndex] < myData[leftChildIndex])
				{
					std::swap(myData[rightChildIndex], myData[currentIndex]);

					currentIndex = rightChildIndex;
				}
				else if (leftChildIndex < static_cast<int>(mySize) && myData[leftChildIndex] < myData[currentIndex])
				{
					std::swap(myData[leftChildIndex], myData[currentIndex]);

					currentIndex = leftChildIndex;
				}
				else
				{
					break;
				}
			}
		}

		return myLargestElement;
	}
}