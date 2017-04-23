#pragma once

#include "PopupButton.h"

namespace RioGui
{

	// Push button with a popup to tweak a color value
	class RIO_EXPORT ColorPicker : public PopupButton
	{
	protected:
		std::function<void(const RioCore::Color4&)> callback;
		ColorWheel* colorWheel;
		Button* pickButton;
	public:
		ColorPicker(Widget* parent, const RioCore::Color4& color = RioCore::Color4(1.0f, 0.0f, 0.0f, 1.0f));

		// Set the change callback
		std::function<void(const RioCore::Color4&)> getCallback() const
		{
			return this->callback;
		}

		void setCallback(const std::function<void(const RioCore::Color4&)>& callback)
		{
			this->callback = callback;
		}

		RioCore::Color4 getColor() const;
		void setColor(const RioCore::Color4& color);
	};

} // namespace RioGui
