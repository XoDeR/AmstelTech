#pragma once

#include "Widget.h"
#include <functional>

namespace RioGui
{

	class RIO_EXPORT ColorWheel : public Widget
	{
	private:
		enum Region
		{
			None = 0,
			InnerTriangle = 1,
			OuterCircle = 2,
			Both = 3
		};
	protected:
		float hue;
		float white;
		float black;
		Region dragRegion = None;
		std::function<void(const Color &)> callback;
	public:
		ColorWheel(Widget* parent, const Color& color = Color(1.0f, 0.0f, 0.0f, 1.0f));

		std::function<void(const Color &)> getCallback() const
		{
			return this->callback;
		}

		void setCallback(const std::function<void(const Color &)>& callback)
		{
			this->callback = callback;
		}

		Color getColor() const;
		void setColor(const Color& color);

		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
		virtual bool mouseButtonEvent(const RioCore::Vector2& p, int button, bool down, int modifiers) override;
		virtual bool mouseDragEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel, int button, int modifiers) override;
	
		Color hue2rgb(float h) const;
		Region adjustPosition(const RioCore::Vector2& p, Region consideredRegions = Both);
	};

} // namespace RioGui
