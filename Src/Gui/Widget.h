#pragma once

#include "Object.h"

#include "Core/Math/MathTypes.h"
#include "Core/Math/Vector2.h"

#include <vector>
#include <functional> // almost all derived classes use std::function

#if defined(__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GL_GLEXT_PROTOTYPES
#endif

namespace RioGui
{
	enum class Cursor;

	class RIO_EXPORT Widget : public Object
	{
	protected:
		Widget* parent = nullptr;
		IntrusivePtr<Theme> theme = nullptr;
		IntrusivePtr<Layout> layout = nullptr;
		std::string stringId;
		RioCore::Vector2 position = RioCore::VECTOR2_ZERO;
		RioCore::Vector2 size = RioCore::VECTOR2_ZERO;
		RioCore::Vector2 fixedSize = RioCore::VECTOR2_ZERO;
		std::vector<Widget*> childrenList;
		bool isVisible = true;
		bool isEnabled = true;
		bool isFocused = false;
		bool isInMouseFocus = false;
		std::string tooltip; // ""
		int fontSize = -1;
		Cursor cursor = Cursor::Arrow;
	public:
		explicit Widget(Widget* parent);

		Widget* getParent()
		{
			return this->parent;
		}

		const Widget* getParent() const
		{
			return this->parent;
		}

		void setParent(Widget* parent)
		{
			this->parent = parent;
		}

		Layout* getLayout()
		{
			return this->layout;
		}

		const Layout* getLayout() const
		{
			return this->layout.get();
		}

		void setLayout(Layout* layout)
		{
			this->layout = layout;
		}

		Theme* getTheme()
		{
			return this->theme;
		}

		const Theme* getTheme() const
		{
			return this->theme.get();
		}

		virtual void setTheme(Theme* theme);

		const RioCore::Vector2& getPosition() const
		{
			return this->position;
		}

		void setPosition(const RioCore::Vector2& pos)
		{
			this->position = pos;
		}

		RioCore::Vector2 getAbsolutePosition() const
		{
			return this->parent ? (getParent()->getAbsolutePosition() + this->position) : this->position;
		}

		const RioCore::Vector2& getSize() const
		{
			return this->size;
		}

		void setSize(const RioCore::Vector2& size)
		{
			this->size = size;
		}

		int getWidth() const
		{
			return this->size.x;
		}

		void setWidth(int width)
		{
			this->size.x = width;
		}

		int getHeight() const
		{
			return this->size.y;
		}

		void setHeight(int height)
		{
			this->size.y = height;
		}

		// If nonzero, components of the fixed size attribute override any values
		// computed by a layout generator associated with this widget. Note that
		// just setting the fixed size alone is not enough to actually change its
		// size; this is done with a call to setSize or a call to performLayout() in the parent widget
		void setFixedSize(const RioCore::Vector2& fixedSize)
		{
			this->fixedSize = fixedSize;
		}

		const RioCore::Vector2& getFixedSize() const
		{
			return this->fixedSize;
		}

		int getFixedWidth() const
		{
			return this->fixedSize.x;
		}

		int getFixedHeight() const
		{
			return this->fixedSize.y;
		}

		void setFixedWidth(int width)
		{
			this->fixedSize.x = width;
		}

		void setFixedHeight(int height)
		{
			this->fixedSize.y = height;
		}

		// Return whether or not the widget is currently visible (assuming all parents are visible)
		bool getIsVisible() const
		{
			return this->isVisible;
		}

		// Set whether or not the widget is currently visible (assuming all parents are visible)
		void setVisible(bool visible)
		{
			this->isVisible = visible;
		}

		// Check if this widget is currently visible, taking parent widgets into account
		bool checkIfVisibleRecursive() const
		{
			bool visible = true;
			const Widget* widget = this;
			while (widget)
			{
				visible = widget->getIsVisible();
				widget = widget->getParent();
			}
			return visible;
		}

		// Return the number of child widgets
		int getChildCount() const
		{
			return (int)this->childrenList.size();
		}

		// Return the list of child widgets of the current widget
		const std::vector<Widget*>& getChildrenList() const
		{
			return this->childrenList;
		}

		// Add a child widget to the current widget at
		// the specified index.
		//
		// This function almost never needs to be called by hand,
		// since the constructor of Widget automatically
		// adds the current widget to its parent
		virtual void addChild(int index, Widget* widget);

		// Convenience function which appends a widget at the end
		void addChild(Widget* widget);

		// Remove a child widget by index
		void removeChild(int index);

		// Remove a child widget by value
		void removeChild(const Widget* widget);

		// Retrieves the child at the specific position
		const Widget* getChildAtIndex(int index) const
		{
			return this->childrenList[index];
		}

		// Retrieves the child at the specific position
		Widget* getChildAtIndex(int index)
		{
			return this->childrenList[index];
		}

		// Returns the index of a specific child or -1 if not found
		int getIndexOfChild(Widget* widget) const;

		// Variadic shorthand notation to construct and add a child widget
		template<typename WidgetClass, typename... Args>
		WidgetClass* add(const Args&... args)
		{
			return new WidgetClass(this, args...);
		}

		// Walk up the hierarchy and return the parent window
		Window* getWindow();

		// Associate this widget with an ID value (optional)
		void setId(const std::string& id)
		{
			this->stringId = id;
		}

		// Return the ID value associated with this widget, if any
		const std::string& getStringId() const
		{
			return this->stringId;
		}

		// Return whether or not this widget is currently enabled
		bool getIsEnabled() const
		{
			return this->isEnabled;
		}
		// Set whether or not this widget is currently enabled
		void setEnabled(bool enabled)
		{
			this->isEnabled = enabled;
		}

		// Return whether or not this widget is currently focused
		bool getIsFocused() const
		{
			return this->isFocused;
		}
		// Set whether or not this widget is currently focused
		void setFocused(bool isFocused)
		{
			this->isFocused = isFocused;
		}

		// Request the focus to be moved to this widget
		void requestFocus();

		const std::string& getTooltip() const
		{
			return this->tooltip;
		}

		void setTooltip(const std::string& tooltip)
		{
			this->tooltip = tooltip;
		}

		// Return current font size. If not set the default of the current theme will be returned
		int getFontSize() const;
		
		void setFontSize(int fontSize)
		{
			this->fontSize = fontSize;
		}
		
		// Return whether the font size is explicitly specified for this widget
		bool hasFontSize() const
		{
			return this->fontSize > 0;
		}

		Cursor getCursor() const
		{
			return this->cursor;
		}

		void setCursor(Cursor cursor)
		{
			this->cursor = cursor;
		}

		// Check if the widget contains a certain position
		bool contains(const RioCore::Vector2& p) const
		{
			RioCore::Vector2 d = p - this->position;
			return (d.x >= 0 && d.y >= 0) && (d.x < this->size.x && d.y < this->size.y);
		}

		// Determine the widget located at the given position value (recursive)
		Widget* findWidget(const RioCore::Vector2& p);

		// Handle a mouse button event (default implementation: propagate to children)
		virtual bool mouseButtonEvent(const RioCore::Vector2& p, int button, bool down, int modifiers);

		// Handle a mouse motion event (default implementation: propagate to children)
		virtual bool mouseMotionEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel, int button, int modifiers);

		// Handle a mouse drag event (default implementation: do nothing)
		virtual bool mouseDragEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel, int button, int modifiers);

		// Handle a mouse enter/leave event (default implementation: record this fact, but do nothing)
		virtual bool mouseEnterEvent(const RioCore::Vector2& p, bool enter);

		// Handle a mouse scroll event (default implementation: propagate to children)
		virtual bool scrollEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel);

		// Handle a focus change event (default implementation: record the focus status, but do nothing)
		virtual bool focusEvent(bool focused);

		// Handle a keyboard event (default implementation: do nothing)
		virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);

		// Handle text input (UTF-32 format) (default implementation: do nothing)
		virtual bool keyboardCharacterEvent(unsigned int codepoint);

		// Compute the preferred size of the widget
		virtual RioCore::Vector2 getPreferredSize(GuiRenderContext* guiRenderContext) const;

		// Invoke the associated layout generator to properly place child widgets, if any
		virtual void performLayout(GuiRenderContext* guiRenderContext);
		virtual void draw(GuiRenderContext* guiRenderContext);
	protected:
		virtual ~Widget();
	};

} // namespace RioGui
