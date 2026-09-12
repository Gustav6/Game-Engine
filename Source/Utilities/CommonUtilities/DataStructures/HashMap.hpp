#pragma once

#include <string>
#include <cassert>

#include "MurmurHash.hpp"

namespace CommonUtilities
{
	// Custom Hasher ref : https://stackoverflow.com/questions/67374637/how-to-write-a-custom-hash-function-for-stdunordered-mapt-in-cpp

	template<typename Key>
	struct Hasher;

	template <class Key, class Value>
	class HashMap
	{
		public:
			HashMap(unsigned int aCapacity);
			~HashMap();

			bool Insert(const Key& aKey, const Value& aValue);
			bool Remove(const Key& aKey);

			const Value* Get(const Key& aKey) const;
			Value* Get(const Key& aKey);

		private:
			enum State
			{
				Empty,
				Used,
				Removed
			};

			struct Entry
			{
				State state = State::Empty;
				Value value;
				Key key;
			};

			unsigned int myCapacity;

			Entry* myData;
	};

	template<class Key, class Value>
	inline HashMap<Key, Value>::HashMap(unsigned int aCapacity)
	{
		assert(aCapacity != 0);

		myCapacity = aCapacity;

		myData = new Entry[myCapacity];
	}

	template<class Key, class Value>
	inline HashMap<Key, Value>::~HashMap()
	{
		delete[] myData;
	}

	template<class Key, class Value>
	inline bool HashMap<Key, Value>::Insert(const Key& aKey, const Value & aValue)
	{
		Hasher<Key> hasher;
		uint32_t originalIndex = hasher(aKey) % myCapacity;

		uint32_t fallbackIndex = myCapacity;

		for (uint32_t i = 0; i < myCapacity; i++)
		{
			uint32_t index = (originalIndex + i * i) % myCapacity;
			Entry& entry = myData[index];

			switch (entry.state)
			{
				case State::Empty:
				{
					entry.value = aValue;
					entry.key = aKey;
					entry.state = State::Used;

					return true;
				}
				case State::Removed:
				{
					fallbackIndex = index;

					break;
				}
				case State::Used:
				{
					if (!(entry.key < aKey) && !(aKey < entry.key))
					{
						entry.value = aValue;

						return true;
					}

					break;
				}
			}
		}

		if (fallbackIndex != myCapacity)
		{
			Entry& entry = myData[fallbackIndex];

			entry.value = aValue;
			entry.key = aKey;
			entry.state = State::Used;

			return true;
		}

		return false;
	}

	template<class Key, class Value>
	inline bool HashMap<Key, Value>::Remove(const Key& aKey)
	{
		Hasher<Key> hasher;
		uint32_t originalIndex = hasher(aKey) % myCapacity;

		for (uint32_t i = 0; i < myCapacity; i++)
		{
			uint32_t index = (originalIndex + i * i) % myCapacity;
			Entry& entry = myData[index];

			switch (entry.state)
			{
				case State::Empty:
				{
					return false;
				}
				case State::Removed:
				{
					break;
				}
				case State::Used:
				{
					if (!(entry.key < aKey) && !(aKey < entry.key))
					{
						myData[index].state = State::Removed;

						return true;
					}

					break;
				}
			}
		}

		return false;
	}

	template<class Key, class Value>
	inline const Value* HashMap<Key, Value>::Get(const Key& aKey) const
	{
		Hasher<Key> hasher;
		uint32_t originalIndex = hasher(aKey) % myCapacity;

		for (uint32_t i = 0; i < myCapacity; i++)
		{
			uint32_t index = (originalIndex + i * i) % myCapacity;
			const Entry& entry = myData[index];

			if (entry.state == State::Empty)
			{
				return nullptr;
			}

			if (entry.state == State::Used)
			{
				if (!(entry.key < aKey) && !(aKey < entry.key))
				{
					return &entry.value;
				}
			}
		}

		return nullptr;
	}

	template<class Key, class Value>
	inline Value* HashMap<Key, Value>::Get(const Key& aKey)
	{
		Hasher<Key> hasher;
		uint32_t originalIndex = hasher(aKey) % myCapacity;

		for (uint32_t i = 0; i < myCapacity; i++)
		{
			uint32_t index = (originalIndex + i * i) % myCapacity;
			Entry& entry = myData[index];

			if (entry.state == State::Empty)
			{
				return nullptr;
			}

			if (entry.state == State::Used)
			{
				if (!(entry.key < aKey) && !(aKey < entry.key))
				{
					return &entry.value;
				}
			}
		}

		return nullptr;
	}
}