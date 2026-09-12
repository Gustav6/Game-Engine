#pragma once

#include <cassert>

namespace CommonUtilities
{
	template <class T>
	class Queue
	{
		public:
			Queue();
			~Queue();

			int GetSize() const;

			const T& GetFront() const;
			T& GetFront();

			void Enqueue(const T& aValue);
			T Dequeue();

		private:
			unsigned int myFrontIndex;
			unsigned int myBackIndex;

			unsigned int mySize;
			unsigned int myCapacity;

			T* myData;
	};

	template<class T>
	inline Queue<T>::Queue()
	{
		myFrontIndex = 0;
		myBackIndex = 0;

		mySize = 0;
		myCapacity = 5;

		myData = new T[myCapacity];
	}

	template<class T>
	inline Queue<T>::~Queue()
	{
		delete[] myData;
	}

	template<class T>
	inline int Queue<T>::GetSize() const
	{
		return mySize;
	}

	template<class T>
	inline const T& Queue<T>::GetFront() const
	{
		assert(mySize > 0);

		return myData[0];
	}

	template<class T>
	inline T& Queue<T>::GetFront()
	{
		assert(mySize > 0);

		return myData[0];
	}

	template<class T>
	inline void Queue<T>::Enqueue(const T& aValue)
	{
		if (mySize >= myCapacity)
		{
			T* resizedArray = new T[myCapacity + 5];

			for (int i = 0; i < static_cast<int>(mySize); i++)
			{
				resizedArray[i] = myData[(myFrontIndex + i) % myCapacity];
			}

			delete[] myData;

			myData = resizedArray;
			myCapacity += 5;
			myFrontIndex = 0;
			myBackIndex = mySize;
		}

		myData[myBackIndex] = aValue;
		myBackIndex = (myBackIndex + 1) % myCapacity;
		mySize++;
	}

	template<class T>
	inline T Queue<T>::Dequeue()
	{
		assert(mySize > 0);

		T& myFirst = myData[myFrontIndex];

		myFrontIndex = (myFrontIndex + 1) % myCapacity;
		mySize--;

		return myFirst;
	}
}