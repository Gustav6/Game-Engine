#pragma once

namespace CommonUtilities
{
	template <class T>
	class BSTSet;

	template <class T>
	class BSTSetNode
	{
		public:
			BSTSetNode(const BSTSetNode&) = delete;
			BSTSetNode& operator=(const BSTSetNode&) = delete;

			const T& GetData() const;
			const BSTSetNode<T>* GetLeft() const;
			const BSTSetNode<T>* GetRight() const;

		private:
			BSTSetNode<T>() {}
			BSTSetNode<T>(T aData) : myData(aData), myLeft(nullptr), myRight(nullptr) {}
			friend class BSTSet<T>;

			T myData;

			BSTSetNode<T>* myLeft;
			BSTSetNode<T>* myRight;
	};

	template <class T>
	class BSTSet
	{
		public:
			BSTSet();
			~BSTSet();

			const BSTSetNode<T>* GetRoot() const;

			bool HasElement(const T& aData) const;

			void Insert(const T& aData);
			void Remove(const T& aData);

			void DSWBalance();

		private:
			void TurnToBackbone();

			void Compact(BSTSetNode<T>* aGrandParent, const int aAmountOfRotations);

			bool IsEqual(const T& aLhs, const T& aRhs) const;

			BSTSetNode<T>* myRoot;
	};

#pragma region Node

	template<class T>
	inline const T& BSTSetNode<T>::GetData() const
	{
		return myData;
	}

	template<class T>
	inline const BSTSetNode<T>* BSTSetNode<T>::GetLeft() const
	{
		return myLeft;
	}

	template<class T>
	inline const BSTSetNode<T>* BSTSetNode<T>::GetRight() const
	{
		return myRight;
	}

#pragma endregion

#pragma region Balanced Tree

	template<class T>
	inline BSTSet<T>::BSTSet()
	{
		myRoot = nullptr;
	}

	template<class T>
	inline BSTSet<T>::~BSTSet()
	{
		TurnToBackbone();

		BSTSetNode<T>* currentNode = myRoot;

		while (currentNode != nullptr)
		{
			BSTSetNode<T>* childNode = currentNode->myRight;

			delete currentNode;

			currentNode = childNode;
		}
	}

	template<class T>
	inline const BSTSetNode<T>* BSTSet<T>::GetRoot() const
	{
		return myRoot;
	}

	template<class T>
	inline bool BSTSet<T>::HasElement(const T& aData) const
	{
		BSTSetNode<T>* currentNode = myRoot;

		while (currentNode != nullptr)
		{
			if (IsEqual(currentNode->myData, aData))
			{
				return true;
			}

			if (currentNode->myData < aData)
			{
				currentNode = currentNode->myRight;
			}
			else
			{
				currentNode = currentNode->myLeft;
			}
		}

		return false;
	}

	template<class T>
	inline void BSTSet<T>::Insert(const T& aData)
	{
		BSTSetNode<T>* currentNode = myRoot;
		BSTSetNode<T>* previousNode = nullptr;

		while (currentNode != nullptr)
		{
			previousNode = currentNode;

			if (IsEqual(currentNode->myData, aData))
			{
				return;
			}

			if (currentNode->myData < aData)
			{
				currentNode = currentNode->myRight;
			}
			else 
			{
				currentNode = currentNode->myLeft;
			}
		}

		currentNode = new BSTSetNode<T>(aData);

		if (previousNode == nullptr)
		{
			myRoot = currentNode;
		}
		else
		{
			if (previousNode->myData < aData)
			{
				previousNode->myRight = currentNode;
			}
			else
			{
				previousNode->myLeft = currentNode;
			}
		}
	}

	template<class T>
	inline void BSTSet<T>::Remove(const T& aData)
	{
		BSTSetNode<T>* currentNode = myRoot;
		BSTSetNode<T>* previousNode = nullptr;

		while (currentNode != nullptr)
		{
			if (IsEqual(currentNode->myData, aData))
			{
				if (previousNode != nullptr)
				{
					BSTSetNode<T>* newChild;

					if (currentNode->myRight != nullptr)
					{
						newChild = currentNode->myRight;

						BSTSetNode<T>* leftMostNode = newChild;

						while (leftMostNode->myLeft != nullptr)
						{
							leftMostNode = leftMostNode->myLeft;
						}

						leftMostNode->myLeft = currentNode->myLeft;
					}
					else 
					{
						newChild = currentNode->myLeft;
					}

					if (previousNode->myRight == currentNode)
					{
						previousNode->myRight = newChild;
					}
					else
					{
						previousNode->myLeft = newChild;
					}
				}
				else 
				{
					BSTSetNode<T>* newRoot;

					if (currentNode->myRight != nullptr)
					{
						newRoot = currentNode->myRight;

						BSTSetNode<T>* leftMostNode = newRoot;

						while (leftMostNode->myLeft != nullptr)
						{
							leftMostNode = leftMostNode->myLeft;
						}

						leftMostNode->myLeft = currentNode->myLeft;
					}
					else
					{
						newRoot = currentNode->myLeft;
					}

					myRoot = newRoot;
				}

				delete currentNode;

				return;
			}

			previousNode = currentNode;

			if (currentNode->myData < aData)
			{
				currentNode = currentNode->myRight;
			}
			else
			{
				currentNode = currentNode->myLeft;
			}
		}
	}

	template<class T>
	inline void BSTSet<T>::DSWBalance()
	{
		TurnToBackbone();

		int size = 0;

		{
			BSTSetNode<T>* currentNode = myRoot;

			while (currentNode != nullptr)
			{
				size++;
				currentNode = currentNode->myRight;
			}
		}

		BSTSetNode<T>* grandParent = new BSTSetNode<T>();
		grandParent->myLeft = nullptr;
		grandParent->myRight = myRoot;
		myRoot = grandParent;

		int m = static_cast<int>(std::pow(2, (std::floor(std::log2(size + 1)))) - 1);

		Compact(grandParent, size - m);

		for (m = m / 2; m > 0; m /= 2)
		{
			Compact(grandParent, m);
		}

		myRoot = grandParent->myRight;
		delete grandParent;
	}

	template<class T>
	inline void BSTSet<T>::Compact(BSTSetNode<T>* aGrandParent, const int aAmountOfRotations)
	{
		BSTSetNode<T>* previousNode = aGrandParent;
		BSTSetNode<T>* currentNode = aGrandParent->myRight;
		BSTSetNode<T>* rightNode = nullptr;

		for (int rotationIndex = 0; rotationIndex < aAmountOfRotations; rotationIndex++)
		{
			if (currentNode == nullptr)
			{
				break;
			}

			rightNode = currentNode->myRight;

			if (rightNode == nullptr)
			{
				break;
			}

			currentNode->myRight = rightNode->myLeft;

			rightNode->myLeft = currentNode;

			previousNode->myRight = rightNode;
			previousNode = rightNode;
			currentNode = rightNode->myRight;
		}
	}

	template<class T>
	inline void BSTSet<T>::TurnToBackbone()
	{
		BSTSetNode<T>* parentNode = myRoot;
		BSTSetNode<T>* grandParentNode = nullptr;
		BSTSetNode<T>* childNode = nullptr;

		while (parentNode != nullptr)
		{
			childNode = parentNode->myLeft;

			if (childNode == nullptr)
			{
				grandParentNode = parentNode;
				parentNode = parentNode->myRight;
			}
			else
			{
				parentNode->myLeft = childNode->myRight;
				childNode->myRight = parentNode;

				if (grandParentNode != nullptr)
				{
					grandParentNode->myRight = childNode;
				}
				else 
				{
					myRoot = childNode;
				}

				parentNode = childNode;
			}
		}
	}

	template<class T>
	inline bool BSTSet<T>::IsEqual(const T& aLhs, const T& aRhs) const
	{
		if (!(aLhs < aRhs) && !(aRhs < aLhs))
		{
			return true;
		}

		return false;
	}

#pragma endregion
}