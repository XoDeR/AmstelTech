#pragma once

#include "Core/Memory/Memory.h"

#include <cstring> // memcpy

namespace Rio
{

template <typename T1, typename T2, int T1Aware, int T2Aware>
struct Pair
{
};

template <typename T1, typename T2>
struct Pair<T1, T2, 0, 0>
{
	ALLOCATOR_AWARE;

	T1 first;
	T2 second;

	Pair(T1& f, T2& s)
		: first(f)
		, second(s)
	{
	}

	Pair(Allocator& /*a*/)
	{
	}
};

template <typename T1, typename T2>
struct Pair<T1, T2, 1, 0>
{
	ALLOCATOR_AWARE;

	T1 first;
	T2 second;

	Pair(T1& f, T2& s)
		: first(f)
		, second(s)
	{
	}

	Pair(Allocator& a)
		: first(a)
	{
	}
};

template <typename T1, typename T2>
struct Pair<T1, T2, 0, 1>
{
	ALLOCATOR_AWARE;

	T1 first;
	T2 second;

	Pair(T1& f, T2& s)
		: first(f)
		, second(s)
	{
	}

	Pair(Allocator& a)
		: second(a)
	{
	}
};

template <typename T1, typename T2>
struct Pair<T1, T2, 1, 1>
{
	ALLOCATOR_AWARE;

	T1 first;
	T2 second;

	Pair(T1& f, T2& s)
		: first(f)
		, second(s)
	{
	}

	Pair(Allocator& a)
		: first(a)
		, second(a)
	{
	}
};

#define PAIR(first, second) Pair<first, second, IS_ALLOCATOR_AWARE(first), IS_ALLOCATOR_AWARE(second)>

template <typename T1, typename T2>
inline void swap(Pair<T1, T2, 0, 0>& a, Pair<T1, T2, 0, 0>& b)
{
	char c[sizeof(a)];
	memcpy(c, &a, sizeof(a));
	memcpy(&a, &b, sizeof(a));
	memcpy(&b, &c, sizeof(a));
}

template <typename T1, typename T2>
inline void swap(Pair<T1, T2, 0, 1>& a, Pair<T1, T2, 0, 1>& b)
{
	char c[sizeof(a)];
	memcpy(c, &a, sizeof(a));
	memcpy(&a, &b, sizeof(a));
	memcpy(&b, &c, sizeof(a));
}

template <typename T1, typename T2>
inline void swap(Pair<T1, T2, 1, 0>& a, Pair<T1, T2, 1, 0>& b)
{
	char c[sizeof(a)];
	memcpy(c, &a, sizeof(a));
	memcpy(&a, &b, sizeof(a));
	memcpy(&b, &c, sizeof(a));
}

template <typename T1, typename T2>
inline void swap(Pair<T1, T2, 1, 1>& a, Pair<T1, T2, 1, 1>& b)
{
	char c[sizeof(a)];
	memcpy(c, &a, sizeof(a));
	memcpy(&a, &b, sizeof(a));
	memcpy(&b, &c, sizeof(a));
}

} // namespace Rio
