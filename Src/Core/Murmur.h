#pragma once

#include "Core/Types.h"

namespace Rio
{

uint32_t murmur32(const void* key, uint32_t len, uint32_t seed);
uint64_t murmur64(const void* key, uint32_t len, uint64_t seed);

} // namespace Rio
