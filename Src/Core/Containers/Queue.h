#pragma once

#include "Core/Containers/Array.h"
#include "Core/Containers/Types.h"
#include "Core/Error/Error.h"

#include <cstring> // memcpy

namespace Rio
{

namespace QueueFn
{
	// Returns whether the queue is empty
	template<typename T> bool getIsEmpty(const Queue<T>& q);

	// Returns the number of items in the queue
	template<typename T> uint32_t getCount(const Queue<T>& q);

	// Returns the number of items the queue can hold before a resize must occur
	template<typename T> uint32_t getSpaceBeforeResize(const Queue<T>& q);

	// Increase or decrease the capacity of the queue
	// Old items will be copied to the newly created queue.
	// If the new <capacity> is smaller than the previous one, the queue will be truncated
	template<typename T> void increaseCapacity(Queue<T>& q, uint32_t capacity);

	// Grows the queue to contain at least <minCapacity> items
	// If <minCapacity> is set to 0, the queue automatically determines the new capacity 
	// based on its size at the time of call
	template<typename T> void grow(Queue<T>& q, uint32_t minCapacity);

	// Appends an <item> to the back of the queue
	template<typename T> void pushBack(Queue<T>& q, const T& item);

	// Removes the last item from the queue
	template<typename T> void popBack(Queue<T>& q);

	// Appends an <item> to the front of the queue
	template<typename T> void pushFront(Queue<T>& q, const T& item);

	// Removes the first item from the queue
	template<typename T> void popFront(Queue<T>& q);

	// Appends <n> <items> to the back of the queue
	template<typename T> void push(Queue<T>& q, const T* items, uint32_t n);

	// Removes <n> items from the front of the queue
	template<typename T> void pop(Queue<T>& q, uint32_t n);

	// Clears the content of the queue
	// Does not free memory nor call destructors,
	// Only zeroes the number of items in the queue for efficiency
	template<typename T> void clear(Queue<T>& q);

	template<typename T> T* begin(Queue<T>& q);
	template<typename T> const T* begin(const Queue<T>& q);
	template<typename T> T* end(Queue<T>& q);
	template<typename T> const T* end(const Queue<T>& q);

	template<typename T> T& getFront(Queue<T>& q);
	template<typename T> const T& getFront(const Queue<T>& q);
	template<typename T> T& getBack(Queue<T>& q);
	template<typename T> const T& getBack(const Queue<T>& q);

} // namespace QueueFn

namespace QueueFn
{
	template <typename T>
	inline bool getIsEmpty(const Queue<T>& q)
	{
		return q.size == 0;
	}

	template <typename T>
	inline uint32_t getCount(const Queue<T>& q)
	{
		return q.size;
	}

	template <typename T>
	inline uint32_t getSpaceBeforeResize(const Queue<T>& q)
	{
		return ArrayFn::getCount(q.dataArray) - q.size;
	}

	template <typename T>
	inline void increaseCapacity(Queue<T>& q, uint32_t capacity)
	{
		uint32_t oldSize = ArrayFn::getCount(q.dataArray);

		ArrayFn::resize(q.dataArray, capacity);

		if (q.read + q.size > oldSize)
		{
			memmove(ArrayFn::begin(q.dataArray) + capacity - (oldSize - q.read), ArrayFn::begin(q.dataArray) + q.read, (oldSize - q.read) * sizeof(T));
			q.read += (capacity - oldSize);
		}
	}

	template <typename T>
	inline void grow(Queue<T>& q, uint32_t minCapacity)
	{
		uint32_t newCapacity = ArrayFn::getCount(q.dataArray) * 2 + 1;

		if (newCapacity < minCapacity)
		{
			newCapacity = minCapacity;
		}

		increaseCapacity(q, newCapacity);
	}

	template <typename T>
	inline void pushBack(Queue<T>& q, const T& item)
	{
		if (getSpaceBeforeResize(q) == 0)
		{
			grow(q, 0);
		}

		q[q.size] = item;

		++q.size;
	}

	template <typename T>
	inline void popBack(Queue<T>& q)
	{
		RIO_ASSERT(q.size > 0, "The queue is empty");
		--q.size;
	}

	template <typename T>
	inline void pushFront(Queue<T>& q, const T& item)
	{
		if (getSpaceBeforeResize(q) == 0)
		{
			grow(q, 0);
		}

		q.read = (q.read - 1 + ArrayFn::getCount(q.dataArray)) % ArrayFn::getCount(q.dataArray);

		q[0] = item;

		++q.size;
	}

	template <typename T>
	inline void popFront(Queue<T>& q)
	{
		RIO_ASSERT(q.size > 0, "The queue is empty");

		q.read = (q.read + 1) % ArrayFn::getCount(q.dataArray);
		--q.size;
	}

	template <typename T>
	inline void push(Queue<T>& q, const T* items, uint32_t n)
	{
		if (q.getSpaceBeforeResize() < n)
		{
			q.grow(q.getCount() + n);
		}

		const uint32_t size = ArrayFn::getCount(q.dataArray);
		const uint32_t insert = (q.read + q.size) % size;

		uint32_t toInsert = n;
		if (insert + toInsert > size)
		{
			toInsert = size - insert;
		}

		memcpy(ArrayFn::begin(q.dataArray) + insert, items, toInsert * sizeof(T));

		q.size += toInsert;
		items += toInsert;
		n -= toInsert;
		memcpy(ArrayFn::begin(q.dataArray), items, n * sizeof(T));

		q.size += n;
	}

	template <typename T>
	inline void pop(Queue<T>& q, uint32_t n)
	{
		RIO_ASSERT(q.size > 0, "The queue is empty");

		q.read = (q.read + n) % ArrayFn::getCount(q.dataArray);
		q.size -= n;
	}

	template <typename T>
	inline void clear(Queue<T>& q)
	{
		q.read = 0;
		q.size = 0;
	}

	template <typename T>
	inline T* begin(Queue<T>& q)
	{
		return ArrayFn::begin(q.dataArray) + q.read;
	}

	template <typename T>
	inline const T* begin(const Queue<T>& q)
	{
		return ArrayFn::begin(q.dataArray) + q.read;
	}

	template <typename T>
	inline T* end(Queue<T>& q)
	{
		const uint32_t end = q.read + q.size;
		return end >= ArrayFn::getCount(q.dataArray) ? ArrayFn::end(q.dataArray) : ArrayFn::begin(q.dataArray) + end;
	}

	template <typename T>
	inline const T* end(const Queue<T>& q)
	{
		const uint32_t end = q.read + q.size;
		return end >= ArrayFn::getCount(q.dataArray) ? ArrayFn::end(q.dataArray) : ArrayFn::begin(q.dataArray) + end;
	}

	template <typename T>
	inline T& getFront(Queue<T>& q)
	{
		RIO_ASSERT(q.size > 0, "The queue is empty");
		return q.dataArray[q.read];
	}

	template <typename T>
	inline const T& getFront(const Queue<T>& q)
	{
		RIO_ASSERT(q.size > 0, "The queue is empty");
		return q.dataArray[q.read];
	}

	template <typename T>
	inline T& getBack(Queue<T>& q)
	{
		RIO_ASSERT(q.size > 0, "The queue is empty");
		return q[q.size - 1];
	}

	template <typename T>
	inline const T& getBack(const Queue<T>& q)
	{
		RIO_ASSERT(q.size > 0, "The queue is empty");
		return q[q.size - 1];
	}

} // namespace QueueFn

template <typename T>
inline Queue<T>::Queue(Allocator& a)
	: dataArray(a)
{
}

template <typename T>
inline T& Queue<T>::operator[](uint32_t index)
{
	return dataArray[(read + index) % ArrayFn::getCount(dataArray)];
}

template <typename T>
inline const T& Queue<T>::operator[](uint32_t index) const
{
	return dataArray[(read + index) % ArrayFn::getCount(dataArray)];
}

} // namespace Rio
