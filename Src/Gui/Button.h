#pragma once

#include "Widget.h"

#include <functional>

namespace RioGui
{
	class RIO_EXPORT Button : public Widget
	{
	public:
		enum Flags
		{
			NormalButton = (1 << 0), // 1
			RadioButton = (1 << 1), // 2
			ToggleButton = (1 << 2), // 4
			PopupButton = (1 << 3)  // 8
		};

		enum class IconPosition
		{
			Left,
			LeftCentered,
			RightCentered,
			Right
		};
	protected:
		std::string caption;
		int icon;
		IconPosition iconPosition = IconPosition::LeftCentered;
		bool isPushed = false;
		int buttonFlags = NormalButton;
		RioCore::Color4 backgroundColor = { 0, 0, 0, 0 };
		RioCore::Color4 textColor = {0, 0, 0, 0};
		std::function<void()> callback;
		std::function<void(bool)> changeCallback;
		std::vector<Button*> buttonGroup;
	public:
		Button(Widget* parent, const std::string& caption = "Untitled", int icon = 0);

		const std::string& getCaption() const
		{
			return this->caption;
		}

		void setCaption(const std::string& caption)
		{
			this->caption = caption;
		}

		const RioCore::Color4& getBackgroundColor() const
		{
			return this->backgroundColor;
		}

		void setBackgroundColor(const RioCore::Color4& backgroundColor)
		{
			this->backgroundColor = backgroundColor;
		}

		const RioCore::Color4& getTextColor() const
		{
			return this->textColor;
		}

		void setTextColor(const RioCore::Color4& textColor)
		{
			this->textColor = textColor;
		}

		int getIcon() const
		{
			return this->icon;
		}

		void setIcon(int icon)
		{
			this->icon = icon;
		}

		int getButtonFlags() const
		{
			return this->buttonFlags;
		}

		void setFlags(int buttonFlags)
		{
			this->buttonFlags = buttonFlags;
		}

		IconPosition getIconPosition() const
		{
			return this->iconPosition;
		}

		void setIconPosition(IconPosition iconPosition)
		{
			this->iconPosition = iconPosition;
		}

		bool getIsPushed() const
		{
			return this->isPushed;
		}

		void setIsPushed(bool pushed)
		{
			this->isPushed = pushed;
		}

		std::function<void()> getCallback() const
		{
			return this->callback;
		}

		void setCallback(const std::function<void()>& callback)
		{
			this->callback = callback;
		}

		// for toggle buttons
		std::function<void(bool)> setChangeCallback() const
		{
			return this->changeCallback;
		}

		void setChangeCallback(const std::function<void(bool)>& callback)
		{
			this->changeCallback = callback;
		}

		// for radio buttons
		void setButtonGroup(const std::vector<Button*>& buttonGroup)
		{
			this->buttonGroup = buttonGroup;
		}

		const std::vector<Button*>& getButtonGroup() const
		{
			return this->buttonGroup;
		}

		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual bool mouseButtonEvent(const RioCore::Vector2& p, int button, bool down, int modifiers) override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
	protected:
		// Determine whether an icon ID is a font-based icon
		inline bool getIsFontIcon(int value)
		{
			return value >= 1024;
		}
	};

} // namespace RioGui
