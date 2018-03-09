#include "Core/Murmur.h"

namespace Rio
{

// MurmurHash2, by Austin Appleby
//
// This code makes a few assumptions about how your machine behaves
//
// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
//
// And it has a few limitations:
//
// 1. It will not work incrementally
// 2. It will not produce the same results on little-endian and big-endian machines
uint32_t murmur32(const void* key, uint32_t len, uint32_t seed)
{
	// 'm' and 'r' are mixing constants generated offline
	// They're not really 'magic', they just happen to work well
	const uint32_t m = 0x5bd1e995;
	const int32_t r = 24;

	// Initialize the hash to a 'random' value
	uint32_t h = seed ^ len;

	// Mix 4 bytes at a time into the hash
	const uint8_t* data = (const uint8_t*)key;

	while(len >= 4)
	{
		uint32_t k = *(uint32_t*)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array
	switch(len)
	{
		case 3: h ^= data[2] << 16; // Fallthrough
		case 2: h ^= data[1] << 8;  // Fallthrough
		case 1: h ^= data[0];       // Fallthrough
			h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few bytes are well-incorporated
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

uint64_t murmur64(const void* key, uint32_t len, uint64_t seed)
{
	const uint64_t m = 0xc6a4a7935bd1e995ull;
	const int32_t r = 47;

	uint64_t h = seed ^ (len * m);

	const uint64_t* data = (const uint64_t*)key;
	const uint64_t* end = data + (len/8);

	while(data != end)
	{
		uint64_t k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const uint8_t* data2 = (const uint8_t*)data;

	switch(len & 7)
	{
		case 7: h ^= uint64_t(data2[6]) << 48; // Fallthrough
		case 6: h ^= uint64_t(data2[5]) << 40; // Fallthrough
		case 5: h ^= uint64_t(data2[4]) << 32; // Fallthrough
		case 4: h ^= uint64_t(data2[3]) << 24; // Fallthrough
		case 3: h ^= uint64_t(data2[2]) << 16; // Fallthrough
		case 2: h ^= uint64_t(data2[1]) << 8;  // Fallthrough
		case 1: h ^= uint64_t(data2[0]);       // Fallthrough
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

} // namespace Rio
