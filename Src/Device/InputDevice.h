#pragma once

#include "Core/Math/Types.h"
#include "Core/Memory/Types.h"
#include "Core/Strings/StringId.h"
#include "Core/Types.h"

namespace Rio
{

// Represents a generic input device
struct InputDevice
{
	bool isConnected = false;
	uint8_t buttonsCount = 0;
	uint8_t axesCount = 0;
	uint8_t _last_button = 0;

	uint8_t* lastStateList = nullptr; // buttonsCount
	uint8_t* stateList = nullptr; // buttonsCount
	Vector3* axesList = nullptr; // axesCount

	const char** buttonNameMap = nullptr; // buttonsCount
	const char** axisNameMap = nullptr; // axesCount

	StringId32* buttonHashList = nullptr; // buttonsCount
	StringId32* axesHashList = nullptr; // axesCount

	char* inputDeviceName = nullptr; // getStrLen32(name) + 1

	// Returns the name of the input device
	const char* getInputDeviceName() const;

	// Returns whether the input device is connected and functioning
	bool getIsConnected() const;

	// Returns the number of buttons of the input device
	uint8_t getButtonsCount() const;

	// Returns the number of axes of the input device
	uint8_t getAxesCount() const;

	// Returns whether the button <id> is pressed in the current frame
	bool getIsButtonPressed(uint8_t id) const;

	// Returns whether the button <id> is released in the current frame
	bool getIsButtonReleased(uint8_t id) const;

	// Returns whether any button is pressed in the current frame
	bool getIsAnyButtonPressed() const;

	// Returns whether any button is released in the current frame
	bool getIsAnyButtonReleased() const;

	// Returns the value of the axis <id>
	Vector3 getAxisById(uint8_t id) const;

	// Returns the name of the button <id> or nullptr if no matching button is found
	const char* getButtonName(uint8_t id);

	// Returns the name of the axis <id> of nullptr if no matching axis is found
	const char* getAxisName(uint8_t id);

	// Returns the id of the button <name> or UINT8_MAX if no matching button is found
	uint8_t getButtonIdByName(StringId32 name);

	// Returns the id of the axis <name> of UINT8_MAX if no matching axis is found
	uint8_t getAxisIdByName(StringId32 name);

	void setButtonPressedState(uint8_t i, bool state);

	void setAxis(uint8_t i, const Vector3& value);

	void update();
};

namespace InputDeviceFn
{
	// Creates a new input device
	InputDevice* create(Allocator& a, const char* name, uint8_t buttonsCount, uint8_t axesCount, const char** buttonNameList, const char** axesNameList);

	// Destroys the input device <id>
	void destroy(Allocator& a, InputDevice& id);

} // namespace InputDeviceFn

} // namespace Rio
