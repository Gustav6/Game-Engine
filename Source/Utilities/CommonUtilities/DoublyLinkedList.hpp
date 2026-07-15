#pragma once

namespace CommonUtilities
{
	template <class T>
	class DoublyLinkedList;

	template <class T>
	class DoublyLinkedListNode
	{
	public:
		DoublyLinkedListNode<T>(const DoublyLinkedListNode<T>&) = delete;
		DoublyLinkedListNode<T>& operator=(const DoublyLinkedListNode<T>&) = delete;

		const T& GetValue() const;
		T& GetValue();

		DoublyLinkedListNode<T>* GetNext() const;
		DoublyLinkedListNode<T>* GetPrevious() const;

	private:
		friend class DoublyLinkedList<T>;

		DoublyLinkedListNode(const T& aValue);
		~DoublyLinkedListNode() {}

		DoublyLinkedListNode* myNext;
		DoublyLinkedListNode* myPrevious;

		T myData;
	};

	template <class T>
	class DoublyLinkedList
	{
		public:
			DoublyLinkedList();
			~DoublyLinkedList();

			int GetSize() const;

			DoublyLinkedListNode<T>* GetFirst();
			DoublyLinkedListNode<T>* GetLast();

			void InsertFirst(const T& aValue);
			void InsertLast(const T& aValue);
			void InsertBefore(DoublyLinkedListNode<T>* aNode, const T& aValue);
			void InsertAfter(DoublyLinkedListNode<T>* aNode, const T& aValue);

			void Remove(DoublyLinkedListNode<T>* aNode);

			DoublyLinkedListNode<T>* FindFirst(const T& aValue);
			DoublyLinkedListNode<T>* FindLast(const T& aValue);

			bool RemoveFirst(const T& aValue);
			bool RemoveLast(const T& aValue);

		private:
			DoublyLinkedListNode<T>* myHead;
			DoublyLinkedListNode<T>* myTail;

			int mySize;
	};

#pragma region Node

	template<class T>
	inline const T& DoublyLinkedListNode<T>::GetValue() const
	{
		return myData;
	}

	template<class T>
	inline T& DoublyLinkedListNode<T>::GetValue()
	{
		return myData;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedListNode<T>::GetNext() const
	{
		return myNext;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedListNode<T>::GetPrevious() const
	{
		return myPrevious;
	}

	template<class T>
	inline DoublyLinkedListNode<T>::DoublyLinkedListNode(const T& aValue)
	{
		myNext = nullptr;
		myPrevious = nullptr;
		myData = aValue;
	}

#pragma endregion

#pragma region List

	template<class T>
	inline DoublyLinkedList<T>::DoublyLinkedList()
	{
		myHead = nullptr;
		myTail = nullptr;
		mySize = 0;
	}

	template<class T>
	inline DoublyLinkedList<T>::~DoublyLinkedList()
	{
		DoublyLinkedListNode<T>* node = myHead;

		while (node != nullptr)
		{
			DoublyLinkedListNode<T>* temp = node;
			node = node->myNext;

			delete temp;
		}
	}

	template<class T>
	inline int DoublyLinkedList<T>::GetSize() const
	{
		return mySize;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedList<T>::GetFirst()
	{
		return myHead;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedList<T>::GetLast()
	{
		return myTail;
	}

	template<class T>
	inline void DoublyLinkedList<T>::InsertFirst(const T& aValue)
	{
		DoublyLinkedListNode<T>* newHead = new DoublyLinkedListNode<T>(aValue);

		newHead->myNext = myHead;

		if (myHead != nullptr)
		{
			myHead->myPrevious = newHead;
		}

		myHead = newHead;

		if (myTail == nullptr)
		{
			myTail = myHead;
		}

		mySize++;
	}

	template<class T>
	inline void DoublyLinkedList<T>::InsertLast(const T& aValue)
	{
		DoublyLinkedListNode<T>* newTail = new DoublyLinkedListNode<T>(aValue);

		newTail->myPrevious = myTail;

		if (myTail != nullptr)
		{
			myTail->myNext = newTail;
		}

		myTail = newTail;

		if (myHead == nullptr)
		{
			myHead = myTail;
		}

		mySize++;
	}

	template<class T>
	inline void DoublyLinkedList<T>::InsertBefore(DoublyLinkedListNode<T>* aNode, const T& aValue)
	{
		if (aNode != nullptr)
		{
			DoublyLinkedListNode<T>* node = new DoublyLinkedListNode<T>(aValue);

			node->myPrevious = aNode->myPrevious;

			if (aNode != myHead)
			{
				aNode->myPrevious->myNext = node;
			}
			else
			{
				myHead = node;
			}

			node->myNext = aNode;

			aNode->myPrevious = node;

			mySize++;
		}
	}

	template<class T>
	inline void DoublyLinkedList<T>::InsertAfter(DoublyLinkedListNode<T>* aNode, const T& aValue)
	{
		if (aNode != nullptr)
		{
			DoublyLinkedListNode<T>* node = new DoublyLinkedListNode<T>(aValue);

			node->myNext = aNode->myNext;

			if (aNode != myTail)
			{
				aNode->myNext->myPrevious = node;
			}
			else
			{
				myTail = node;
			}

			node->myPrevious = aNode;

			aNode->myNext = node;

			mySize++;
		}
	}

	template<class T>
	inline void DoublyLinkedList<T>::Remove(DoublyLinkedListNode<T>* aNode)
	{
		if (aNode != nullptr)
		{
			if (aNode->myPrevious != nullptr)
			{
				aNode->myPrevious->myNext = aNode->myNext;
			}
			else
			{
				myHead = aNode->myNext;
			}

			if (aNode->myNext != nullptr)
			{
				aNode->myNext->myPrevious = aNode->myPrevious;
			}
			else
			{
				myTail = aNode->myPrevious;
			}

			delete aNode;
			mySize--;
		}
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedList<T>::FindFirst(const T& aValue)
	{
		DoublyLinkedListNode<T>* node = myHead;

		while (node != nullptr)
		{
			if (node->GetValue() == aValue)
			{
				return node;
			}

			node = node->myNext;
		}

		return nullptr;
	}

	template<class T>
	inline DoublyLinkedListNode<T>* DoublyLinkedList<T>::FindLast(const T& aValue)
	{
		DoublyLinkedListNode<T>* node = myHead;
		DoublyLinkedListNode<T>* last = nullptr;

		while (node != nullptr)
		{
			if (node->GetValue() == aValue)
			{
				last = node;
			}

			node = node->myNext;
		}

		return last;
	}

	template<class T>
	inline bool DoublyLinkedList<T>::RemoveFirst(const T& aValue)
	{
		DoublyLinkedListNode<T>* node = myHead;

		while (node != nullptr)
		{
			if (node->GetValue() == aValue)
			{
				Remove(node);
				return true;
			}

			node = node->myNext;
		}

		return false;
	}

	template<class T>
	inline bool DoublyLinkedList<T>::RemoveLast(const T& aValue)
	{
		DoublyLinkedListNode<T>* node = myHead;
		DoublyLinkedListNode<T>* last = nullptr;

		while (node != nullptr)
		{
			if (node->GetValue() == aValue)
			{
				last = node;
			}

			node = node->myNext;
		}

		if (last != nullptr)
		{
			Remove(last);
			return true;
		}

		return false;
	}

#pragma endregion
}