#pragma once

#include "Widget.h"

#include "Core/Containers/ContainerTypes.h"
#include "Core/Containers/Array.h"
#include "Core/Memory/Allocator.h"

namespace RioGui
{

	// Simple graph widget for showing a function plot
	class RIO_EXPORT Graph : public Widget
	{
	protected:
		std::string caption;
		std::string graphHeader;
		std::string graphFooter;

		RioCore::Color4 backgroundColor = { 20, 20, 20, 128 };
		RioCore::Color4 foregroundColor = { 255, 192, 0, 128 };
		RioCore::Color4 textColor = { 240, 240, 240, 192 };
		RioCore::Array<float> graphValueList;
	public:
		Graph(RioCore::Allocator& a, Widget* parent, const std::string& caption = "Untitled");

		const std::string& getCaption() const
		{
			return this->caption;
		}

		void setCaption(const std::string& caption)
		{
			this->caption = caption;
		}

		const std::string& getHeader() const
		{
			return this->graphHeader;
		}

		void setHeader(const std::string& header)
		{
			this->graphHeader = header;
		}

		const std::string& getFooter() const
		{
			return this->graphFooter;
		}

		void setFooter(const std::string& footer)
		{
			this->graphFooter = footer;
		}

		const RioCore::Color4& getBackgroundColor() const
		{
			return this->backgroundColor;
		}

		void setBackgroundColor(const RioCore::Color4& backgroundColor)
		{
			this->backgroundColor = backgroundColor;
		}

		const RioCore::Color4& getForegroundColor() const
		{
			return this->foregroundColor;
		}

		void setForegroundColor(const RioCore::Color4& foregroundColor)
		{
			this->foregroundColor = foregroundColor;
		}

		const RioCore::Color4& getTextColor() const
		{
			return this->textColor;
		}

		void setTextColor(const RioCore::Color4& textColor)
		{
			this->textColor = textColor;
		}

		const RioCore::Array<float>& getGraphValues() const
		{
			return this->graphValueList;
		}

		RioCore::Array<float>& getGraphValues()
		{
			return this->graphValueList;
		}

		void setValues(const RioCore::Array<float>& values)
		{
			this->graphValueList = values;
		}

		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
	};

} // namespace RioGui
