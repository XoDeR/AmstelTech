#pragma once

#include "Core/Functional.h"
#include "Core/Memory/Types.h"
#include "Core/Pair.h"
#include "Core/Types.h"

namespace Rio
{

// Dynamic array of POD items
// Does not call constructors/destructors, uses memcpy to move stuff around
template <typename T>
struct Array
{
	ALLOCATOR_AWARE;

	Allocator* allocator = nullptr;
	uint32_t capacity = 0;
	uint32_t size = 0;
	T* data = nullptr;

	Array(Allocator& a);
	Array(const Array<T>& other);
	~Array();
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;
	Array<T>& operator=(const Array<T>& other);
};

using Buffer = Array<char>;

// Dynamic array of objects
// Calls constructors and destructors
// If your data is POD, use Array<T> instead
template <typename T>
struct Vector
{
	ALLOCATOR_AWARE;

	Allocator* allocator = nullptr;
	uint32_t capacity = 0;
	uint32_t size = 0;
	T* data = nullptr;

	Vector(Allocator& a);
	Vector(const Vector<T>& other);
	~Vector();
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;
	const Vector<T>& operator=(const Vector<T>& other);
};

// Circular buffer double-ended queue of POD items
template <typename T>
struct Queue
{
	ALLOCATOR_AWARE;

	uint32_t read = 0;
	uint32_t size = 0;
	Array<T> dataArray;

	Queue(Allocator& a);
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;
};

// Map from key to value
// Uses a Vector internally, so not suited for performance-critical stuff
template <typename TKey, typename TValue>
struct Map
{
	ALLOCATOR_AWARE;

	struct Node
	{
		ALLOCATOR_AWARE;

		PAIR(TKey, TValue) pair;
		uint32_t left = 0;
		uint32_t right = 0;
		uint32_t parent = 0;
		uint32_t color = 0;

		Node(Allocator& a)
			: pair(a)
		{
		}
	};

	uint32_t root = 0;
	uint32_t sentinel = 0;
	Vector<Node> dataVector;

	Map(Allocator& a);
	const TValue& operator[](const TKey& key) const;
};

template <typename TKey, typename TValue, typename THash = Hash<TKey>>
struct HashMap
{
	ALLOCATOR_AWARE;

	using Entry = PAIR(TKey, TValue);

	struct Index
	{
		uint32_t hash = 0;
		uint32_t index = 0;
	};

	Allocator* allocator = nullptr;
	uint32_t capacity = 0;
	uint32_t size = 0;
	uint32_t mask = 0;
	Index* index = nullptr;
	Entry* data = nullptr;

	HashMap(Allocator& a);
	~HashMap();
	const TValue& operator[](const TKey& key) const;
};

// Vector of sorted items
// Items are not automatically sorted, 
// Need to call SortMapFn::sort() whenever you are done inserting/removing items
template <typename TKey, typename TValue, typename Compare = Less<TKey>>
struct SortMap
{
	ALLOCATOR_AWARE;

	using Entry = PAIR(TKey, TValue);

	Vector<Entry> dataVector;
#if RIO_DEBUG
	bool isSorted = true;
#endif

	SortMap(Allocator& a);
};

} // namespace Rio
