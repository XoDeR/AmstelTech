#pragma once

#include "Widget.h"

namespace RioGui
{

	class RIO_EXPORT ProgressBar : public Widget
	{
	protected:
		float progressValue = 0.0f;
	public:
		ProgressBar(Widget* parent);

		float getValue()
		{
			return this->progressValue;
		}

		void setValue(float progressValue)
		{
			this->progressValue = progressValue;
		}

		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
	};

} // namespace RioGui
