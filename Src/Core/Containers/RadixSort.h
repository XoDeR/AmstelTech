// Copyright (c) 2016, 2017 Volodymyr Syvochka
#pragma once

#include "Core/Base/RioCommon.h"

namespace RioCore
{
#define RIO_RADIXSORT_BITS 11
#define RIO_RADIXSORT_HISTOGRAM_SIZE (1<<RIO_RADIXSORT_BITS)
#define RIO_RADIXSORT_BIT_MASK (RIO_RADIXSORT_HISTOGRAM_SIZE-1)

	inline void radixSort(uint32_t* __restrict _keys, uint32_t* __restrict _tempKeys, uint32_t _size)
	{
		uint32_t* __restrict keys = _keys;
		uint32_t* __restrict tempKeys = _tempKeys;

		uint32_t histogram[RIO_RADIXSORT_HISTOGRAM_SIZE];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 3; ++pass)
		{
			memset(histogram, 0, sizeof(uint32_t)*RIO_RADIXSORT_HISTOGRAM_SIZE);

			bool sorted = true;
			{
				uint32_t key = keys[0];
				uint32_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key)
				{
					key = keys[ii];
					uint16_t index = (key >> shift)&RIO_RADIXSORT_BIT_MASK;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted)
			{
				goto done;
			}

			uint32_t offset = 0;
			for (uint32_t ii = 0; ii < RIO_RADIXSORT_HISTOGRAM_SIZE; ++ii)
			{
				uint32_t count = histogram[ii];
				histogram[ii] = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				uint32_t key = keys[ii];
				uint16_t index = (key >> shift)&RIO_RADIXSORT_BIT_MASK;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
			}

			uint32_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			shift += RIO_RADIXSORT_BITS;
		}

	done:
		if (0 != (pass & 1))
		{
			// Odd number of passes needs to do copy to the destination.
			memcpy(_keys, _tempKeys, _size * sizeof(uint32_t));
		}
	}

	template <typename Ty>
	inline void radixSort(uint32_t* __restrict _keys, uint32_t* __restrict _tempKeys, Ty* __restrict _values, Ty* __restrict _tempValues, uint32_t _size)
	{
		uint32_t* __restrict keys = _keys;
		uint32_t* __restrict tempKeys = _tempKeys;
		Ty* __restrict values = _values;
		Ty* __restrict tempValues = _tempValues;

		uint32_t histogram[RIO_RADIXSORT_HISTOGRAM_SIZE];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 3; ++pass)
		{
			memset(histogram, 0, sizeof(uint32_t)*RIO_RADIXSORT_HISTOGRAM_SIZE);

			bool sorted = true;
			{
				uint32_t key = keys[0];
				uint32_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key)
				{
					key = keys[ii];
					uint16_t index = (key >> shift)&RIO_RADIXSORT_BIT_MASK;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted)
			{
				goto done;
			}

			uint32_t offset = 0;
			for (uint32_t ii = 0; ii < RIO_RADIXSORT_HISTOGRAM_SIZE; ++ii)
			{
				uint32_t count = histogram[ii];
				histogram[ii] = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				uint32_t key = keys[ii];
				uint16_t index = (key >> shift)&RIO_RADIXSORT_BIT_MASK;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
				tempValues[dest] = values[ii];
			}

			uint32_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			Ty* swapValues = tempValues;
			tempValues = values;
			values = swapValues;

			shift += RIO_RADIXSORT_BITS;
		}

	done:
		if (0 != (pass & 1))
		{
			// Odd number of passes needs to do copy to the destination.
			memcpy(_keys, _tempKeys, _size * sizeof(uint32_t));
			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				_values[ii] = _tempValues[ii];
			}
		}
	}

	inline void radixSort(uint64_t* __restrict _keys, uint64_t* __restrict _tempKeys, uint32_t _size)
	{
		uint64_t* __restrict keys = _keys;
		uint64_t* __restrict tempKeys = _tempKeys;

		uint32_t histogram[RIO_RADIXSORT_HISTOGRAM_SIZE];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 6; ++pass)
		{
			memset(histogram, 0, sizeof(uint32_t)*RIO_RADIXSORT_HISTOGRAM_SIZE);

			bool sorted = true;
			{
				uint64_t key = keys[0];
				uint64_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key)
				{
					key = keys[ii];
					uint16_t index = (key >> shift)&RIO_RADIXSORT_BIT_MASK;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted)
			{
				goto done;
			}

			uint32_t offset = 0;
			for (uint32_t ii = 0; ii < RIO_RADIXSORT_HISTOGRAM_SIZE; ++ii)
			{
				uint32_t count = histogram[ii];
				histogram[ii] = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				uint64_t key = keys[ii];
				uint16_t index = (key >> shift)&RIO_RADIXSORT_BIT_MASK;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
			}

			uint64_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			shift += RIO_RADIXSORT_BITS;
		}

	done:
		if (0 != (pass & 1))
		{
			// Odd number of passes needs to do copy to the destination.
			memcpy(_keys, _tempKeys, _size * sizeof(uint64_t));
		}
	}

	template <typename Ty>
	inline void radixSort(uint64_t* __restrict _keys, uint64_t* __restrict _tempKeys, Ty* __restrict _values, Ty* __restrict _tempValues, uint32_t _size)
	{
		uint64_t* __restrict keys = _keys;
		uint64_t* __restrict tempKeys = _tempKeys;
		Ty* __restrict values = _values;
		Ty* __restrict tempValues = _tempValues;

		uint32_t histogram[RIO_RADIXSORT_HISTOGRAM_SIZE];
		uint16_t shift = 0;
		uint32_t pass = 0;
		for (; pass < 6; ++pass)
		{
			memset(histogram, 0, sizeof(uint32_t) * RIO_RADIXSORT_HISTOGRAM_SIZE);

			bool sorted = true;
			{
				uint64_t key = keys[0];
				uint64_t prevKey = key;
				for (uint32_t ii = 0; ii < _size; ++ii, prevKey = key)
				{
					key = keys[ii];
					uint16_t index = (key >> shift)&RIO_RADIXSORT_BIT_MASK;
					++histogram[index];
					sorted &= prevKey <= key;
				}
			}

			if (sorted)
			{
				goto done;
			}

			uint32_t offset = 0;
			for (uint32_t ii = 0; ii < RIO_RADIXSORT_HISTOGRAM_SIZE; ++ii)
			{
				uint32_t count = histogram[ii];
				histogram[ii] = offset;
				offset += count;
			}

			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				uint64_t key = keys[ii];
				uint16_t index = (key >> shift) & RIO_RADIXSORT_BIT_MASK;
				uint32_t dest = histogram[index]++;
				tempKeys[dest] = key;
				tempValues[dest] = values[ii];
			}

			uint64_t* swapKeys = tempKeys;
			tempKeys = keys;
			keys = swapKeys;

			Ty* swapValues = tempValues;
			tempValues = values;
			values = swapValues;

			shift += RIO_RADIXSORT_BITS;
		}

	done:
		if (0 != (pass & 1))
		{
			// Odd number of passes needs to do copy to the destination.
			memcpy(_keys, _tempKeys, _size * sizeof(uint64_t));
			for (uint32_t ii = 0; ii < _size; ++ii)
			{
				_values[ii] = _tempValues[ii];
			}
		}
	}

#undef RIO_RADIXSORT_BITS
#undef RIO_RADIXSORT_HISTOGRAM_SIZE
#undef RIO_RADIXSORT_BIT_MASK

} // namespace RioCore
// Copyright (c) 2016, 2017 Volodymyr Syvochka