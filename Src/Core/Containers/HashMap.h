#pragma once

#include "Core/Containers/Types.h"

#include <algorithm> // std::swap
#include <new>
#include <cstring>  // memcpy

namespace Rio
{

namespace HashMapFn
{
	// Returns the number of items in the map <m>
	template <typename TKey, typename TValue, typename THash> uint32_t getCount(const HashMap<TKey, TValue, THash>& m);

	// Returns the maximum number of items the map <m> can hold
	template <typename TKey, typename TValue, typename THash> uint32_t getCapacity(const HashMap<TKey, TValue, THash>& m);

	// Returns whether the given <key> exists in the map <m>
	template <typename TKey, typename TValue, typename THash> bool has(const HashMap<TKey, TValue, THash>& m, const TKey& key);

	// Returns the value for the given <key> or <deffault> if the key does not exist in the map
	template <typename TKey, typename TValue, typename THash> const TValue& get(const HashMap<TKey, TValue, THash>& m, const TKey& key, const TValue& deffault);

	// Sets the <value> for the <key> in the map
	template <typename TKey, typename TValue, typename THash> void set(HashMap<TKey, TValue, THash>& m, const TKey& key, const TValue& value);

	// Removes the <key> from the map if it exists
	template <typename TKey, typename TValue, typename THash> void remove(HashMap<TKey, TValue, THash>& m, const TKey& key);

	// Removes all the items in the map
	// Calls destructor on the items
	template <typename TKey, typename TValue, typename THash> void clear(HashMap<TKey, TValue, THash>& m);

} // namespace HashMapFn

namespace HashMapInternalFn
{
	const uint32_t END_OF_LIST = 0xffffffffu;
	const uint32_t DELETED = 0x80000000u;
	const uint32_t FREE = 0x00000000u;

	template <typename TKey, typename THash>
	inline uint32_t getHashKey(const TKey& key)
	{
		const THash hash;
		return hash(key); // uses Hash templates from Core/Functional.h
	}

	inline bool getIsDeleted(uint32_t index)
	{
		// MSB set indicates that this hash is a "tombstone"
		return (index >> 31) != 0;
	}

	template <typename TKey, typename TValue, typename THash>
	inline uint32_t getProbeDistance(const HashMap<TKey, TValue, THash>& m, uint32_t hash, uint32_t slotIndex)
	{
		const uint32_t hashI = hash & m.mask;
		return (slotIndex + m.capacity - hashI) & m.mask;
	}

	template <typename TKey, typename TValue, typename THash>
	uint32_t find(const HashMap<TKey, TValue, THash>& m, const TKey& key)
	{
		if (m.size == 0)
		{
			return END_OF_LIST;
		}

		const uint32_t hash = getHashKey<TKey, THash>(key);
		uint32_t hashI = hash & m.mask;
		uint32_t dist = 0;
		for(;;)
		{
			if (m.index[hashI].index == FREE)
			{
				return END_OF_LIST;
			}
			else if (dist > getProbeDistance(m, m.index[hashI].hash, hashI))
			{
				return END_OF_LIST;
			}
			else if (!getIsDeleted(m.index[hashI].index)
				&& m.index[hashI].hash == hash
				&& m.data[hashI].first == key)
			{
				return hashI;
			}

			hashI = (hashI + 1) & m.mask;
			++dist;
		}
	}

	template <typename TKey, typename TValue, typename THash>
	void insert(HashMap<TKey, TValue, THash>& m, uint32_t hash, const TKey& key, const TValue& value)
	{
		PAIR(TKey, TValue) newItem(*m.allocator);
		newItem.first  = key;
		newItem.second = value;

		uint32_t hashI = hash & m.mask;
		uint32_t dist = 0;
		for(;;)
		{
			if (m.index[hashI].index == FREE)
			{
				goto INSERT_AND_RETURN;
			}

			// If the existing element has probed less than us, then swap places with existing elem, 
			// and keep looking for another slot for that element
			uint32_t existingElementProbeDist = getProbeDistance(m, m.index[hashI].hash, hashI);
			if (getIsDeleted(m.index[hashI].index) || existingElementProbeDist < dist)
			{
				if (getIsDeleted(m.index[hashI].index))
				{
					goto INSERT_AND_RETURN;
				}

				std::swap(hash, m.index[hashI].hash);
				m.index[hashI].index = 0x0123abcd;
				swap(newItem, m.data[hashI]);

				dist = existingElementProbeDist;
			}

			hashI = (hashI + 1) & m.mask;
			++dist;
		}

	INSERT_AND_RETURN:
		new (m.data + hashI) typename HashMap<TKey, TValue, THash>::Entry(*m.allocator);
		memcpy(m.data + hashI, &newItem, sizeof(newItem));
		m.index[hashI].hash = hash;
		m.index[hashI].index = 0x0123abcd;
		PAIR(TKey, TValue) empty(*m.allocator);
		memcpy(&newItem, &empty, sizeof(newItem));
	}

	template <typename TKey, typename TValue, typename THash>
	void rehash(HashMap<TKey, TValue, THash>& m, uint32_t newCapacity)
	{
		typedef typename HashMap<TKey, TValue, THash>::Entry Entry;
		typedef typename HashMap<TKey, TValue, THash>::Index Index;

		HashMap<TKey, TValue, THash> nm(*m.allocator);
		nm.index = (Index*)nm.allocator->allocate(newCapacity * sizeof(Index), alignof(Index));
		nm.data = (Entry*)nm.allocator->allocate(newCapacity * sizeof(Entry), alignof(Entry));

		// Flag all elements as free
		for (uint32_t i = 0; i < newCapacity; ++i)
		{
			nm.index[i].hash = 0;
			nm.index[i].index = FREE;
		}

		nm.capacity = newCapacity;
		nm.size = m.size;
		nm.mask = newCapacity - 1;

		for (uint32_t i = 0; i < m.capacity; ++i)
		{
			typename HashMap<TKey, TValue, THash>::Entry& e = m.data[i];
			const uint32_t hash = m.index[i].hash;
			const uint32_t index = m.index[i].index;

			if (index != FREE && !getIsDeleted(index))
			{
				HashMapInternalFn::insert(nm, hash, e.first, e.second);
			}
		}

		HashMap<TKey, TValue, THash> empty(*m.allocator);
		m.~HashMap<TKey, TValue, THash>();
		memcpy(&m, &nm, sizeof(HashMap<TKey, TValue, THash>));
		memcpy(&nm, &empty, sizeof(HashMap<TKey, TValue, THash>));
	}

	template <typename TKey, typename TValue, typename THash>
	void grow(HashMap<TKey, TValue, THash>& m)
	{
		const uint32_t newCapacity = (m.capacity == 0 ? 16 : m.capacity * 2);
		rehash(m, newCapacity);
	}

	template <typename TKey, typename TValue, typename THash>
	bool getIsFull(const HashMap<TKey, TValue, THash>& m)
	{
		return m.size >= m.capacity * 0.9f;
	}

} // namespace HashMapInternalFn

namespace HashMapFn
{
	template <typename TKey, typename TValue, typename THash>
	uint32_t getCount(const HashMap<TKey, TValue, THash>& m)
	{
		return m.size;
	}

	template <typename TKey, typename TValue, typename THash>
	uint32_t getCapacity(const HashMap<TKey, TValue, THash>& m)
	{
		return m.capacity;
	}

	template <typename TKey, typename TValue, typename THash>
	bool has(const HashMap<TKey, TValue, THash>& m, const TKey& key)
	{
		return HashMapInternalFn::find(m, key) != HashMapInternalFn::END_OF_LIST;
	}

	template <typename TKey, typename TValue, typename THash>
	const TValue& get(const HashMap<TKey, TValue, THash>& m, const TKey& key, const TValue& deffault)
	{
		const uint32_t i = HashMapInternalFn::find(m, key);
		if (i == HashMapInternalFn::END_OF_LIST)
		{
			return deffault;
		}
		else
		{
			return m.data[i].second;
		}
	}

	template <typename TKey, typename TValue, typename THash>
	void set(HashMap<TKey, TValue, THash>& m, const TKey& key, const TValue& value)
	{
		if (m.capacity == 0)
		{
			HashMapInternalFn::grow(m);
		}

		// Find or make
		const uint32_t i = HashMapInternalFn::find(m, key);
		if (i == HashMapInternalFn::END_OF_LIST)
		{
			HashMapInternalFn::insert(m, HashMapInternalFn::getHashKey<TKey, THash>(key), key, value);
			++m.size;
		}
		else
		{
			m.data[i].second = value;
		}
		if (HashMapInternalFn::getIsFull(m))
		{
			HashMapInternalFn::grow(m);
		}
	}

	template <typename TKey, typename TValue, typename THash>
	void remove(HashMap<TKey, TValue, THash>& m, const TKey& key)
	{
		const uint32_t i = HashMapInternalFn::find(m, key);
		if (i == HashMapInternalFn::END_OF_LIST)
		{
			return;
		}

		m.data[i].~Pair();
		m.index[i].index |= HashMapInternalFn::DELETED;
		--m.size;
	}

	template <typename TKey, typename TValue, typename THash>
	void clear(HashMap<TKey, TValue, THash>& m)
	{
		for (uint32_t i = 0; i < m.capacity; ++i)
		{
			if (m.index[i].index == 0x0123abcd)
			{
				m.data[i].~Pair();
			}
			m.index[i].index = HashMapInternalFn::FREE;
		}

		m.size = 0;
	}

} // namespace HashMapFn

template <typename TKey, typename TValue, typename THash>
HashMap<TKey, TValue, THash>::HashMap(Allocator& a)
	: allocator(&a)
{
}

template <typename TKey, typename TValue, typename THash>
HashMap<TKey, TValue, THash>::~HashMap()
{
	for (uint32_t i = 0; i < capacity; ++i)
	{
		if (index[i].index == 0x0123abcd)
		{
			data[i].~Pair();
		}
	}

	allocator->deallocate(index);
	allocator->deallocate(data);
}

template <typename TKey, typename TValue, typename THash>
const TValue& HashMap<TKey, TValue, THash>::operator[](const TKey& key) const
{
	return HashMapFn::get(*this, key, TValue());
}

} // namespace Rio
