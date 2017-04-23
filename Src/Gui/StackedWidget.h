#pragma once

#include "Widget.h"

namespace RioGui
{

	class RIO_EXPORT StackedWidget : public Widget
	{
	private:
		int selectedIndex = -1;
	public:
		StackedWidget(Widget* parent);

		void setSelectedIndex(int index);
		int getSelectedIndex() const;

		virtual void performLayout(GuiRenderContext* guiRenderContext) override;
		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual void addChild(int index, Widget* widget) override;
	};

} // namespace RioGui
