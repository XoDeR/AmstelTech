#pragma once

#include "PopupButton.h"

namespace RioGui
{

	// Push button with a popup to tweak a color value
	class RIO_EXPORT ColorPicker : public PopupButton
	{
	protected:
		std::function<void(const Color &)> callback;
		ColorWheel* colorWheel;
		Button* pickButton;
	public:
		ColorPicker(Widget* parent, const Color& color = Color(1.0f, 0.0f, 0.0f, 1.0f));

		// Set the change callback
		std::function<void(const Color&)> getCallback() const
		{
			return this->callback;
		}

		void setCallback(const std::function<void(const Color &)>& callback)
		{
			this->callback = callback;
		}

		Color getColor() const;
		void setColor(const Color& color);
	};

} // namespace RioGui
