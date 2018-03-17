#pragma once

#include "Core/Strings/Types.h"

namespace Rio
{

namespace ErrorFn
{
	// Fills stringStream with the current call stack
	void callstack(StringStream& stringStream);

} // namespace ErrorFn

} // namespace Rio
