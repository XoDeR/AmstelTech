#pragma once

#include "PopupButton.h"

namespace RioGui
{
	// Simple combo box widget based on a popup button
	class RIO_EXPORT ComboBox : public PopupButton
	{
	protected:
		std::vector<std::string> itemList;
		std::vector<std::string> itemShortList;
		std::function<void(int)> callback;
		int selectedIndex = 0;
	public:
		// Create an empty combo box
		ComboBox(Widget* parent);

		// Create a new combo box with the given items
		ComboBox(Widget* parent, const std::vector<std::string>& items);

		// Create a new combo box with the given items, providing both short and
		// long descriptive labels for each item
		ComboBox(Widget* parent, const std::vector<std::string>& items, const std::vector<std::string>& itemsShort);

		std::function<void(int)> getCallback() const
		{
			return this->callback;
		}

		void setCallback(const std::function<void(int)>& callback)
		{
			this->callback = callback;
		}

		int getSelectedIndex() const
		{
			return this->selectedIndex;
		}

		void setSelectedIndex(int idx);

		void setItems(const std::vector<std::string>& items, const std::vector<std::string>& itemsShort);
		
		void setItems(const std::vector<std::string>& items)
		{
			setItems(items, items);
		}

		const std::vector<std::string>& getItemList() const
		{
			return this->itemList;
		}

		const std::vector<std::string>& getItemShortList() const
		{
			return this->itemShortList;
		}

		virtual bool scrollEvent(const RioCore::Vector2& p, const RioCore::Vector2& rel) override;
	};

} // namespace RioGui
