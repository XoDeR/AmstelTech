#pragma once

#include "Core/Containers/Types.h"

namespace Rio
{

struct DynamicString;
struct FixedString;
struct StringId32;
struct StringId64;

using ResourceId = StringId64 ;

// Stream of characters
using StringStream = Array<char>;

} // namespace Rio
