#pragma once

#include "Core/Base/RioCommon.h"

namespace RioCore
{
	inline uint16_t endianSwap(uint16_t in)
	{
		return (in >> 8) | (in << 8);
	}
	
	inline uint32_t endianSwap(uint32_t in)
	{
		return (in >> 24) | (in << 24)
			 | ((in & 0x00ff0000) >> 8) | ((in & 0x0000ff00) << 8)
			 ;
	}

	inline uint64_t endianSwap(uint64_t in)
	{
		return (in >> 56) | (in << 56)
			 | ((in & uint64_t(0x00ff000000000000)) >> 40) | ((in & uint64_t(0x000000000000ff00)) << 40)
			 | ((in & uint64_t(0x0000ff0000000000)) >> 24) | ((in & uint64_t(0x0000000000ff0000)) << 24)
			 | ((in & uint64_t(0x000000ff00000000)) >> 8)  | ((in & uint64_t(0x00000000ff000000)) << 8)
			 ;
	}

	inline int16_t endianSwap(int16_t in)
	{
		return (int16_t)endianSwap((uint16_t)in);
	}

	inline int32_t endianSwap(int32_t in)
	{
		return (int32_t)endianSwap((uint32_t)in);
	}

	inline int64_t endianSwap(int64_t in)
	{
		return (int64_t)endianSwap((uint64_t)in);
	}

	// Input argument is encoded as little endian, convert it if neccessary
	// depending on host CPU endianess
	template <typename Ty>
	inline Ty toLittleEndian(const Ty in)
	{
#if RIO_CPU_ENDIAN_BIG
		return endianSwap(in);
#else
		return in;
#endif // RIO_CPU_ENDIAN_BIG
	}

	// Input argument is encoded as big endian, convert it if neccessary
	// depending on host CPU endianess.
	template <typename Ty>
	inline Ty toBigEndian(const Ty in)
	{
#if RIO_CPU_ENDIAN_LITTLE
		return endianSwap(in);
#else
		return in;
#endif // RIO_CPU_ENDIAN_LITTLE
	}

	// If _littleEndian is true, converts input argument to from little endian
	// to host CPU endiness.
	template <typename Ty>
	inline Ty toHostEndian(const Ty in, bool fromLittleEndian)
	{
#if RIO_CPU_ENDIAN_LITTLE
		return fromLittleEndian ? in : endianSwap(in);
#else
		return fromLittleEndian ? endianSwap(in) : in;
#endif // RIO_CPU_ENDIAN_LITTLE
	}

} // namespace RioCore
