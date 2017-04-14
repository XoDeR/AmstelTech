#pragma once

#include "Button.h"
#include "Popup.h"
#include "Entypo.h"

namespace RioGui
{
	// Button which launches a popup widget.
	class RIO_EXPORT PopupButton : public Button
	{
	protected:
		Popup* popup;
		int chevronIcon = ENTYPO_ICON_CHEVRON_SMALL_RIGHT;
	public:
		PopupButton(Widget* parent, const std::string& caption = "Untitled", int buttonIcon = 0);

		void setChevronIcon(int chevronIcon)
		{
			this->chevronIcon = chevronIcon;
		}

		int getChevronIcon() const
		{
			return this->chevronIcon;
		}

		void setSide(Popup::Side popupSide);

		Popup::Side getSide() const
		{
			return this->popup->getSide();
		}

		Popup* getPopup()
		{
			return this->popup;
		}

		const Popup* getPopup() const
		{
			return this->popup;
		}

		virtual void draw(GuiRenderContext* guiRenderContext) override;
		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual void performLayout(GuiRenderContext* guiRenderContext) override;

	};

} // namespace RioGui
