#pragma once

#include "Core/Memory/Types.h"
#include "Core/Types.h"

namespace Rio
{

struct DisplayMode
{
	uint32_t id = UINT32_MAX;
	uint32_t width = 0;
	uint32_t height = 0;
};

// Display interface
struct Display
{
	// Fills <modes> with all available display modes
	virtual void getDisplayModeList(Array<DisplayMode>& modes) = 0;

	// Sets the mode <id>
	// The initial display mode is automatically reset when the program terminates
	virtual void setMode(uint32_t id) = 0;
};

namespace DisplayFn
{
	// Creates a new display
	Display* create(Allocator& a);

	// Destroys the display <d>
	void destroy(Allocator& a, Display& d);

} // namespace DisplayFn

} // namespace Rio
