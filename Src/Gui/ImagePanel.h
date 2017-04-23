#pragma once

#include "Widget.h"

namespace RioGui
{
	// Shows a number of square-shaped icons
	class RIO_EXPORT ImagePanel : public Widget
	{
	protected:
		std::vector<std::pair<int, std::string>> imageList;
		std::function<void(int)> callback;
		int thumbSize = 64;
		int spacing = 10;
		int margin = 10;
		int mouseIndex = -1;
	public:
		ImagePanel(Widget* parent);

		void setImages(const std::vector<std::pair<int, std::string>>& data) 
		{ 
			this->imageList = data;
		}

		const std::vector<std::pair<int, std::string>>& getImageList() const 
		{ 
			return this->imageList;
		}

		std::function<void(int)> getCallback() const 
		{ 
			return this->callback;
		}

		void setCallback(const std::function<void(int)>& callback) 
		{ 
			this->callback = callback;
		}

		virtual bool mouseMotionEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel, int button, int modifiers) override;
		virtual bool mouseButtonEvent(const RioCore::Vector2& p, int button, bool down, int modifiers) override;
		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const override;
		virtual void draw(GuiRenderContext* guiRenderContext) override;
	protected:
		RioCore::Vector2 getGridSize() const;
		int getIndexForPosition(const RioCore::Vector2& p) const;
	};

} // namespace RioGui
