#pragma once

#include "Core/Types.h"

namespace Rio
{

struct IpAddress
{
	uint8_t a = 0;
	uint8_t b = 0;
	uint8_t c = 0;
	uint8_t d = 0;

	// Returns the IP address as packed 32-bit integer
	uint32_t getAddressAsUint32() const
	{
		return (uint32_t(a) << 24) | (uint32_t(b) << 16) | (uint32_t(c) << 8) | uint32_t(d);
	}
};

const IpAddress IP_ADDRESS_LOOPBACK = { 127, 0, 0, 1 };

} // namespace Rio
