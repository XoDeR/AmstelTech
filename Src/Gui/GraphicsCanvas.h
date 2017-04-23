#pragma once

#include <iostream>

#include "Widget.h"
#include "GlUtil.h"

namespace RioGui
{

	// Canvas widget that can be used to display arbitrary graphics content
	// This is useful to display and manipulate 3D objects as part of an interactive application
	// The implementation uses scissoring to ensure that rendered objects don't spill into neighboring widgets
	// Usage: override drawGL() in subclasses to provide custom drawing code
	class RIO_EXPORT GraphicsCanvas : public Widget
	{
	protected:
		Color backgroundColor = RioCore::Vector4{ 128, 128, 128, 255 };
		bool drawBorder = true;
	public:
		GraphicsCanvas(Widget* parent);

		const Color& getBackgroundColor() const
		{
			return this->backgroundColor;
		}

		void setBackgroundColor(const Color& backgroundColor)
		{
			this->backgroundColor = backgroundColor;
		}

		void setDrawBorder(const bool drawBorder)
		{
			this->drawBorder = drawBorder;
		}

		const bool& getDrawBorder() const
		{
			return this->drawBorder;
		}

		virtual void draw(GuiRenderContext* ctx) override;

		virtual void drawGL() 
		{
		}
	protected:
		void drawWidgetBorder(GuiRenderContext* ctx) const;
	};

} // namespace RioGui
