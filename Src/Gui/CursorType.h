#pragma once

namespace RioGui
{

	// Cursor shapes available to use in GLFW.
	enum class Cursor
	{
		Arrow = 0,
		IBeam,
		Crosshair,
		Hand,
		HResize,
		VResize,
		// Not a cursor --- should always be last: enables a loop over the cursor types
		CursorCount
	};
} // namespace RioGui