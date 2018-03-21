#pragma once

#include "Core/Memory/Types.h"
#include "Core/Types.h"

namespace Rio
{

// Window interface
struct Window
{
	// Opens the window
	virtual void open(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t parent) = 0;

	// Closes the window
	virtual void close() = 0;

	// Shows the window
	virtual void show() = 0;

	// Hides the window
	virtual void hide() = 0;

	// Resizes the window to <width> and <height>
	virtual void resize(uint16_t width, uint16_t height) = 0;

	// Moves the window to <x> and <y>
	virtual void move(uint16_t x, uint16_t y) = 0;

	// Minimizes the window
	virtual void minimize() = 0;

	// Restores the window
	virtual void restore() = 0;

	// Returns the title of the window
	virtual const char* getWindowTitle() = 0;

	// Sets the title of the window
	virtual void setWindowTitle(const char* title) = 0;

	// Returns the native window handle
	virtual void* getWindowHandle() = 0;

	// Sets whether to <show> the cursor
	virtual void setShowCursor(bool show) = 0;

	// Sets whether the window is <fullscreen>
	virtual void setIsFullscreen(bool fullscreen) = 0;

	virtual void rioRendererSetup() = 0;
};

namespace WindowFn
{
	// Creates a new window
	Window* create(Allocator& a);

	// Destroys the window <w>
	void destroy(Allocator& a, Window& w);

} // namespace WindowFn

} // namespace Rio
