#pragma once

#include "Widget.h"

namespace RioGui
{

	// The font and color can be customized
	// When Widget::setFixedWidth() is used, the text is wrapped when it surpasses the specified width
	class RIO_EXPORT Label : public Widget
	{
	protected:
		std::string caption;
		std::string fontName;
		RioCore::Color4 color;
	public:
		Label(Widget* parent, const std::string& caption, const std::string& font = "sans", int fontSize = -1);

		const std::string& getCaption() const
		{
			return this->caption;
		}

		void setCaption(const std::string& caption)
		{
			this->caption = caption;
		}

		// Set the currently active font 
		// 2 are available by default: sans, sans-bold
		void setFont(const std::string& font)
		{
			this->fontName = font;
		}

		// Get the currently active font
		const std::string& getFont() const
		{
			return this->fontName;
		}

		RioCore::Color4 getColor() const
		{
			return this->color;
		}

		void setColor(const RioCore::Color4& color)
		{
			this->color = color;
		}

		virtual void setTheme(Theme* theme) override;

		// Compute the size needed to fully display the label
		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
	};

} // namespace RioGui
