#pragma once

#include <cassert>

namespace CommonUtilities
{
	template <class T>
	class Stack
	{
		public:
			Stack();
			~Stack();

			int GetSize() const;

			const T& GetTop() const;
			T& GetTop();

			void Push(const T& aValue);
			T Pop();

		private:
			unsigned int mySize;
			unsigned int myCapacity;

			T* myData;
	};

	template<class T>
	inline Stack<T>::Stack()
	{
		mySize = 0;
		myCapacity = 5;

		myData = new T[myCapacity];
	}

	template<class T>
	inline Stack<T>::~Stack()
	{
		delete[] myData;
	}

	template<class T>
	inline int Stack<T>::GetSize() const
	{
		return mySize;
	}

	template<class T>
	inline const T& Stack<T>::GetTop() const
	{
		assert(mySize > 0);

		return myData[mySize - 1];
	}

	template<class T>
	inline T& Stack<T>::GetTop()
	{
		assert(mySize > 0);

		return myData[mySize - 1];
	}

	template<class T>
	inline void Stack<T>::Push(const T& aValue)
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

		myData[mySize] = aValue;
		mySize++;
	}

	template<class T>
	inline T Stack<T>::Pop()
	{
		assert(mySize > 0);

		T& data = myData[mySize - 1];

		mySize--;

		return data;
	}
}