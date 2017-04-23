#pragma once

#include "Widget.h"

#include <functional>

namespace RioGui
{
	// Fractional slider widget with mouse control
	class RIO_EXPORT Slider : public Widget
	{
	protected:
		float sliderValue = 0.0f;
		std::function<void(float)> callback;
		std::function<void(float)> finalCallback;
		std::pair<float, float> sliderRange = { 0.0f, 1.0f };
		std::pair<float, float> highlightedRange = { 0.0f, 0.0f };
		Color highlightColor = Color(255, 80, 80, 70);
	public:
		Slider(Widget* parent);

		float getValue() const
		{
			return this->sliderValue;
		}

		void setValue(float value)
		{
			this->sliderValue = value;
		}

		const Color& getHighlightColor() const
		{
			return this->highlightColor;
		}

		void setHighlightColor(const Color& highlightColor)
		{
			this->highlightColor = highlightColor;
		}

		std::pair<float, float> getRange() const
		{
			return this->sliderRange;
		}

		void setRange(std::pair<float, float> range)
		{
			this->sliderRange = range;
		}

		std::pair<float, float> getHighlightedRange() const
		{
			return this->highlightedRange;
		}

		void setHighlightedRange(std::pair<float, float> highlightedRange)
		{
			this->highlightedRange = highlightedRange;
		}

		std::function<void(float)> getCallback() const
		{
			return this->callback;
		}

		void setCallback(const std::function<void(float)>& callback)
		{
			this->callback = callback;
		}

		std::function<void(float)> getFinalCallback() const
		{
			return this->finalCallback;
		}

		void setFinalCallback(const std::function<void(float)>& callback)
		{
			this->finalCallback = callback;
		}

		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* ctx) const override;
		virtual bool mouseDragEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel, int button, int modifiers) override;
		virtual bool mouseButtonEvent(const RioCore::Vector2& p, int button, bool down, int modifiers) override;
		virtual void draw(GuiRenderContext* ctx) override;
	};

} // namespace RioGui
