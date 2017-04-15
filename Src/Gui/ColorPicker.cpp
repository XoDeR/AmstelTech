#include "ColorPicker.h"
#include "Layout.h"
#include "ColorWheel.h"

namespace RioGui
{

	ColorPicker::ColorPicker(Widget* parent, const Color& color) 
		: PopupButton(parent, "")
	{
		setBackgroundColor(color);
		Popup* popup = this->getPopup();
		popup->setLayout(new GroupLayout());

		this->colorWheel = new ColorWheel(popup);
		this->pickButton = new Button(popup, "Pick");
		this->pickButton->setFixedSize(RioCore::Vector2{ 100, 25 });

		PopupButton::setChangeCallback([&](bool)
		{
			setColor(getBackgroundColor());
			this->callback(getBackgroundColor());
		});

		this->colorWheel->setCallback([&](const Color& value)
		{
			this->pickButton->setBackgroundColor(value);
			this->pickButton->setTextColor(value.getContrastingColor());
			this->callback(value);
		});

		this->pickButton->setCallback([&]()
		{
			Color value = this->colorWheel->getColor();
			setIsPushed(false);
			setColor(value);
			this->callback(value);
		});
	}

	Color ColorPicker::getColor() const
	{
		return getBackgroundColor();
	}

	void ColorPicker::setColor(const Color& color)
	{
		// Ignore setColor() calls when the user is currently editing 
		if (!this->isPushed)
		{
			Color fg = color.getContrastingColor();
			setBackgroundColor(color);
			setTextColor(fg);
			this->colorWheel->setColor(color);
			this->pickButton->setBackgroundColor(color);
			this->pickButton->setTextColor(fg);
		}
	}

} // namespace RioGui
