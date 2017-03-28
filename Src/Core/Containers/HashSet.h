// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"

#include <algorithm> // std::swap
#include <new>
#include <string.h>  // memcpy

namespace RioCore
{

	namespace HashSetFn
	{
		template <typename TKey, typename Hash> uint32_t getCount(const HashSet<TKey, Hash>& m);
		// Returns whether the given <key> exists in the map
		template <typename TKey, typename Hash> bool has(const HashSet<TKey, Hash>& m, const TKey& key);
		// Sets the <value> for the <key> in the map
		template <typename TKey, typename Hash> void insert(HashSet<TKey, Hash>& m, const TKey& key);
		// Removes the <key> from the map if it exists
		template <typename TKey, typename Hash> void remove(HashSet<TKey, Hash>& m, const TKey& key);
		// Removes all the items in the map
		// Calls destructor on the items
		template <typename TKey, typename Hash> void clear(HashSet<TKey, Hash>& m);
	} // namespace HashMapFn

	namespace HashSetInternalFn
	{
		const uint32_t END_OF_LIST = 0xffffffffu;
		const uint32_t DELETED = 0x80000000u;
		const uint32_t FREE = 0x00000000u;

		template <typename TKey, class Hash>
		inline uint32_t getHashKey(const TKey& key)
		{
			const Hash hash;
			uint32_t h = hash(key);

			// the most significant byte is used to indicate a deleted element, so clear it
			h &= 0x7fffffffu;

			// Ensure that we never return 0 as a hash,
			// since we use 0 to indicate that the elem has never been used at all
			h |= h == 0u;

			return h;
		}

		inline bool getIsDeleted(uint32_t hash)
		{
			// the most significant byte set indicates that this hash is a "tombstone"
			return (hash >> 31) != 0;
		}

		template <typename TKey, typename Hash>
		inline uint32_t getProbeDistance(const HashSet<TKey, Hash>& m, uint32_t hash, uint32_t slotIndex)
		{
			const uint32_t hashIndex = hash & m.mask;
			return (slotIndex + m.capacity - hashIndex) & m.mask;
		}

		template <typename TKey, typename Hash>
		uint32_t find(const HashSet<TKey, Hash>& m, const TKey& key)
		{
			if (m.size == 0)
			{
				return END_OF_LIST;
			}

			const uint32_t hash = getHashKey<TKey, Hash>(key);
			uint32_t hashIndex = hash & m.mask;
			uint32_t dist = 0;
			for (;;)
			{
				if (m.hashList[hashIndex] == 0)
				{
					return END_OF_LIST;
				}
				else if (dist > getProbeDistance(m, m.hashList[hashIndex], hashIndex))
				{
					return END_OF_LIST;
				}
				else if (m.hashList[hashIndex] == hash && m.data[hashIndex].key == key)
				{
					return hashIndex;
				}

				hashIndex = (hashIndex + 1) & m.mask;
				++dist;
			}
		}

		template <typename TKey, typename Hash>
		void insert(HashSet<TKey, Hash>& m, uint32_t hash, TKey& key)
		{
			// goto is used for efficiency there
			uint32_t hashIndex = hash & m.mask;
			uint32_t distance = 0;
			for (;;)
			{
				if (m.hashList[hashIndex] == FREE)
				{
					goto INSERT_AND_RETURN;
				}

				// If the existing element has probed less than us, then swap places with existing element, 
				// and keep going to find another slot for that element
				uint32_t existingElementProbeDistance = getProbeDistance(m, m.hashList[hashIndex], hashIndex);
				if (existingElementProbeDistance < distance)
				{
					if (getIsDeleted(m.hashList[hashIndex]))
					{
						goto INSERT_AND_RETURN;
					}
					std::swap(hash, m.hashList[hashIndex]);
					std::swap(key, m.data[hashIndex].key);
					distance = existingElementProbeDistance;
				}

				hashIndex = (hashIndex + 1) & m.mask;
				++distance;
			}

		INSERT_AND_RETURN:
			new (m.data + hashIndex) typename HashSet<TKey, Hash>::Entry(*m.allocator);
			m.data[hashIndex].key = key;
			m.hashList[hashIndex] = hash;
		}

		template <typename TKey, typename Hash>
		void rehash(HashSet<TKey, Hash>& m, uint32_t newCapacity)
		{
			using Entry = typename HashSet<TKey, Hash>::Entry;

			HashSet<TKey, Hash> newHashMap(*m.allocator);
			newHashMap.hashList = (uint32_t*)newHashMap.allocator->allocate(newCapacity * sizeof(uint32_t), alignof(uint32_t));
			newHashMap.data = (Entry*)newHashMap.allocator->allocate(newCapacity * sizeof(Entry), alignof(Entry));

			// Flag all elements as free
			for (uint32_t i = 0; i < newCapacity; ++i)
			{
				newHashMap.hashList[i] = FREE;
			}

			newHashMap.capacity = newCapacity;
			newHashMap.size = m.size;
			newHashMap.mask = newCapacity - 1;

			for (uint32_t i = 0; i < m.capacity; ++i)
			{
				typename HashSet<TKey, Hash>::Entry& e = m.data[i];
				const uint32_t hash = m.hashList[i];

				if (hash != FREE && !getIsDeleted(hash))
				{
					HashSetInternalFn::insert(newHashMap, hash, e.key);
				}
			}

			HashSet<TKey, Hash> empty(*m.allocator);
			m.~HashSet<TKey, Hash>();
			memcpy(&m, &newHashMap, sizeof(HashSet<TKey, Hash>));
			memcpy(&newHashMap, &empty, sizeof(HashSet<TKey, Hash>));
		}

		template <typename TKey, typename Hash>
		void grow(HashSet<TKey, Hash>& m)
		{
			const uint32_t newCapacity = (m.capacity == 0 ? 16 : m.capacity * 2);
			rehash(m, newCapacity);
		}

		template <typename TKey, typename Hash>
		bool isFull(const HashSet<TKey, Hash>& m)
		{
			return m.size >= m.capacity * 0.9f;
		}
	} // namespace HashSetInternalFn

	namespace HashSetFn
	{
		template <typename TKey, typename Hash>
		uint32_t getCount(const HashSet<TKey, Hash>& m)
		{
			return m.size;
		}

		template <typename TKey, typename Hash>
		bool has(const HashSet<TKey, Hash>& m, const TKey& key)
		{
			return HashSetInternalFn::find(m, key) != HashMapInternalFn::END_OF_LIST;
		}

		template <typename TKey, typename Hash>
		void insert(HashSet<TKey, Hash>& m, const TKey& key)
		{
			if (m.size == 0)
			{
				HashSetInternalFn::grow(m);
			}

			// Find or make
			const uint32_t i = HashSetInternalFn::find(m, key);
			if (i == HashSetInternalFn::END_OF_LIST)
			{
				HashSetInternalFn::insert(m, HashSetInternalFn::getHashKey<TKey, Hash>(key), const_cast<TKey&>(key));
				++m.size;
			}
			else
			{
				m.data[i].pair.second = value;
			}
			if (HashSetInternalFn::isFull(m) == true)
			{
				HashSetInternalFn::grow(m);
			}
		}

		template <typename TKey, typename Hash>
		void remove(HashSet<TKey, Hash>& m, const TKey& key)
		{
			const uint32_t i = HashSetInternalFn::find(m, key);
			if (i == HashSetInternalFn::END_OF_LIST)
			{
				return;
			}

			m.data[i].~Entry();
			m.hashList[i] |= HashSetInternalFn::DELETED;
			--m.size;
		}

		template <typename TKey, typename Hash>
		void clear(HashSet<TKey, Hash>& m)
		{
			m.size = 0;

			// Flag all elements as free
			for (uint32_t i = 0; i < m.capacity; ++i)
			{
				m.hashList[i] = HashSetInternalFn::FREE;
			}

			for (uint32_t i = 0; i < m.size; ++i)
			{
				m.data[i].~Entry();
			}
		}
	} // namespace HashMapFn

	template <typename TKey, typename Hash>
	HashSet<TKey, Hash>::HashSet(Allocator& a)
		: allocator(&a)
	{
	}

	template <typename TKey, typename Hash>
	HashSet<TKey, Hash>::~HashSet()
	{
		allocator->deallocate(hashList);
		for (uint32_t i = 0; i < size; ++i)
		{
			data[i].~Entry();
		}
		allocator->deallocate(this->data);
	}

} // namespace RioCore
// Copyright (c) 2017 Volodymyr Syvochka