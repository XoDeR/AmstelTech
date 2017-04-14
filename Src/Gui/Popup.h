#pragma once

#include "Window.h"

namespace RioGui
{

	// Popup window for combo boxes, popup buttons, nested dialogs etc
	// Usually the Popup instance is constructed by another widget (e.g. PopupButton) and does not need to be created by hand
	class RIO_EXPORT Popup : public Window
	{
	public:
		enum Side
		{
			Left = 0,
			Right
		};

	protected:
		Window* parentWindow;
		RioCore::Vector2 anchorPosition = RioCore::VECTOR2_ZERO;
		int anchorHeight = 30;
		Side side = Side::Right;
	public:

		// Create a new popup parented to a screen (first argument) and a parent window
		Popup(Widget* parent, Window* parentWindow);

		// Return the anchor position in the parent window; the placement of the popup is relative to it
		void setAnchorPosition(const RioCore::Vector2& anchorPosition)
		{
			this->anchorPosition = anchorPosition;
		}

		// Set the anchor position in the parent window; the placement of the popup is relative to it
		const RioCore::Vector2& getAnchorPosition() const
		{
			return this->anchorPosition;
		}

		// Set the anchor height; this determines the vertical shift relative to the anchor position
		void setAnchorHeight(int anchorHeight)
		{
			this->anchorHeight = anchorHeight;
		}

		// Return the anchor height; this determines the vertical shift relative to the anchor position
		int getAnchorHeight() const
		{
			return this->anchorHeight;
		}

		// Set the side of the parent window at which popup will appear
		void setSide(Side popupSide)
		{
			this->side = popupSide;
		}

		// Return the side of the parent window at which popup will appear
		Side getSide() const
		{
			return this->side;
		}

		// Return the parent window of the popup
		Window* getParentWindow()
		{
			return this->parentWindow;
		}

		// Return the parent window of the popup
		const Window* getParentWindow() const
		{
			return this->parentWindow;
		}

		// Invoke the associated layout generator to properly place child widgets, if any
		virtual void performLayout(GuiRenderContext* guiRenderContext) override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
	protected:
		// Internal helper function to maintain nested window position values
		virtual void refreshRelativePlacement() override;
	};

} // namespace RioGui
