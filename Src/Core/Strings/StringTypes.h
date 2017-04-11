// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"

namespace RioCore
{

struct DynamicString;
struct FixedString;
struct StringId32;
struct StringId64;

using  ResourceId = StringId64;
// Stream of characters
using StringStream = Array<char>;

} // namespace RioCore
// Copyright (c) 2016 Volodymyr Syvochka