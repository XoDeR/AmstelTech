#pragma once

#include "Widget.h"

namespace RioGui
{

	class RIO_EXPORT CheckBox : public Widget
	{
	protected:
		std::string caption;
		bool pushed = false;
		bool checked = false;
		std::function<void(bool)> callback;
	public:
		CheckBox(Widget* parent, const std::string& caption = "Untitled", const std::function<void(bool)>& callback = std::function<void(bool)>());

		const std::string& getCaption() const
		{
			return this->caption;
		}
		void setCaption(const std::string& caption)
		{
			this->caption = caption;
		}

		const bool& getIsChecked() const
		{
			return this->checked;
		}

		void setIsChecked(const bool& checked)
		{
			this->checked = checked;
		}

		const bool& getIsPushed() const
		{
			return this->pushed;
		}

		void setIsPushed(const bool& pushed)
		{
			this->pushed = pushed;
		}

		std::function<void(bool)> getCallback() const
		{
			return this->callback;
		}

		void setCallback(const std::function<void(bool)>& callback)
		{
			this->callback = callback;
		}

		virtual bool mouseButtonEvent(const RioCore::Vector2& p, int button, bool down, int modifiers) override;
		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
	};

} // namespace RioGui
