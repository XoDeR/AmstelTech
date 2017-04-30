#pragma once

#include "Window.h"

namespace RioGui
{

	class RIO_EXPORT MessageDialog : public Window
	{
	public:
		// Classification of the type of message this MessageDialog represents
		enum class Type
		{
			Information,
			Question,
			Warning
		};

	protected:
		std::function<void(int)> callback;
		Label* messageLabel;
	public:
		MessageDialog(Widget* parent, Type type, const std::string& title = "Untitled",
			const std::string& message = "Message", const std::string& buttonText = "OK",
			const std::string& altButtonText = "Cancel", bool altButton = false
		);

		Label* getMessageLabel()
		{
			return this->messageLabel;
		}

		const Label* getMessageLabel() const
		{
			return this->messageLabel;
		}

		std::function<void(int)> getCallback() const
		{
			return this->callback;
		}

		void setCallback(const std::function<void(int)>& callback)
		{
			this->callback = callback;
		}
	
	};

} // namespace RioGui
