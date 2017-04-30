#pragma once

#include "Widget.h"

#include "Device/InputTypes.h"

namespace RioGui
{

	class RIO_EXPORT Window : public Widget
	{
		friend class Popup;
	protected:
		std::string windowTitle;
		Widget* windowButtonPanel = nullptr;
		bool isWindowModal = false;
		bool isWindowDragging = false;
	public:
		Window(Widget* parent, const std::string& title = "Untitled");

		const std::string& getTitle() const
		{
			return this->windowTitle;
		}

		void setTitle(const std::string& title)
		{
			this->windowTitle = title;
		}

		bool getIsModal() const
		{
			return this->isWindowModal;
		}

		void setModal(bool isWindowModal)
		{
			this->isWindowModal = isWindowModal;
		}

		Widget* getButtonPanel();

		void dispose();

		// Center the window in the current Screen
		void center();

		// Draw the window
		virtual void draw(GuiRenderContext* guiRenderContext) override;
		// Handle window drag events
		virtual bool mouseDragEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel, int button, int modifiers) override;
		// Handle mouse events recursively and bring the current window to the top
		virtual bool mouseButtonEvent(const RioCore::Vector2& p, RioDevice::MouseButton::Enum button, bool down, int modifiers) override;
		// Accept scroll events and propagate them to the widget under the mouse cursor
		virtual bool scrollEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel) override;
		// Compute the preferred size of the widget
		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		// Invoke the associated layout generator to properly place child widgets, if any
		virtual void performLayout(GuiRenderContext* guiRenderContext) override;
	protected:
		// Internal helper function to maintain nested window position values; overridden in Popup
		virtual void refreshRelativePlacement();
	};

} // namespace RioGui
