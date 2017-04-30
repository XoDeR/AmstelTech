#pragma once

#include "Widget.h"

namespace RioGui
{

	// Adds a vertical scrollbar around a widget that is too big to fit into a certain area
	class RIO_EXPORT VScrollPanel : public Widget
	{
	protected:
		int childPreferredHeight = 0;
		float scroll = 0.0f;
		bool needToUpdateLayout = false;
	public:
		VScrollPanel(Widget* parent);

		virtual void performLayout(GuiRenderContext* guiRenderContext) override;
		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual bool mouseDragEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel, int button, int modifiers) override;
		virtual bool scrollEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel) override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
	};

} // namespace RioGui
