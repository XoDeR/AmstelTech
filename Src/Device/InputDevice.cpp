#include "Device/InputDevice.h"

#include "Core/Error/Error.h"
#include "Core/Memory/Allocator.h"
#include "Core/Memory/Memory.h"
#include "Core/Strings/String.h"
#include "Core/Strings/StringId.h"

#include <cstring> // strcpy, memset

namespace Rio
{

const char* InputDevice::getInputDeviceName() const
{
	return inputDeviceName;
}

bool InputDevice::getIsConnected() const
{
	return isConnected;
}

uint8_t InputDevice::getButtonsCount() const
{
	return buttonsCount;
}

uint8_t InputDevice::getAxesCount() const
{
	return axesCount;
}

bool InputDevice::getIsButtonPressed(uint8_t id) const
{
	return id < buttonsCount
		? (~lastStateList[id] & stateList[id]) != 0
		: false
		;
}

bool InputDevice::getIsButtonReleased(uint8_t id) const
{
	return id < buttonsCount
		? (lastStateList[id] & ~stateList[id]) != 0
		: false
		;
}

bool InputDevice::getIsAnyButtonPressed() const
{
	return getIsButtonPressed(_last_button);
}

bool InputDevice::getIsAnyButtonReleased() const
{
	return getIsButtonReleased(_last_button);
}

Vector3 InputDevice::getAxisById(uint8_t id) const
{
	return id < axesCount
		? axesList[id]
		: VECTOR3_ZERO
		;
}

const char* InputDevice::getButtonName(uint8_t id)
{
	return id < buttonsCount
		? buttonNameMap[id]
		: nullptr
		;
}

const char* InputDevice::getAxisName(uint8_t id)
{
	return id < axesCount
		? axisNameMap[id]
		: nullptr
		;
}

uint8_t InputDevice::getButtonIdByName(StringId32 name)
{
	for (uint32_t i = 0; i < buttonsCount; ++i)
	{
		if (buttonHashList[i] == name)
		{
			return i;
		}
	}

	return UINT8_MAX;
}

uint8_t InputDevice::getAxisIdByName(StringId32 name)
{
	for (uint32_t i = 0; i < axesCount; ++i)
	{
		if (axesHashList[i] == name)
		{
			return i;
		}
	}

	return UINT8_MAX;
}

void InputDevice::setButtonPressedState(uint8_t i, bool state)
{
	RIO_ASSERT(i < buttonsCount, "Index out of bounds");
	_last_button = i;
	stateList[i] = state;
}

void InputDevice::setAxis(uint8_t i, const Vector3& value)
{
	RIO_ASSERT(i < axesCount, "Index out of bounds");
	axesList[i] = value;
}

void InputDevice::update()
{
	memcpy(this->lastStateList, this->stateList, sizeof(uint8_t)*(this->buttonsCount));
}

namespace InputDeviceFn
{
	InputDevice* create(Allocator& a, const char* name, uint8_t buttonsCount, uint8_t axesCount, const char** buttonNameList, const char** axesNameList)
	{
		const uint32_t size = 0
			+ sizeof(InputDevice) + alignof(InputDevice)
			+ sizeof(uint8_t)*buttonsCount *2 + alignof(uint8_t)
			+ sizeof(Vector3)*axesCount + alignof(Vector3)
			+ sizeof(char*)*buttonsCount + alignof(char*)
			+ sizeof(char*)*axesCount + alignof(char*)
			+ sizeof(StringId32)*buttonsCount + alignof(StringId32)
			+ sizeof(StringId32)*axesCount + alignof(StringId32)
			+ getStrLen32(name) + 1 + alignof(char)
			;

		InputDevice* inputDevice = (InputDevice*)a.allocate(size);

		inputDevice->isConnected = false;
		inputDevice->buttonsCount = buttonsCount;
		inputDevice->axesCount = axesCount;
		inputDevice->_last_button = 0;

		inputDevice->lastStateList = (uint8_t*)&inputDevice[1];
		inputDevice->stateList = (uint8_t*)Memory::getAlignedToTop(inputDevice->lastStateList + buttonsCount, alignof(uint8_t));
		inputDevice->axesList = (Vector3*)Memory::getAlignedToTop(inputDevice->stateList + buttonsCount, alignof(Vector3));
		inputDevice->buttonNameMap = (const char**)Memory::getAlignedToTop(inputDevice->axesList + axesCount, alignof(const char*));
		inputDevice->axisNameMap = (const char**)Memory::getAlignedToTop(inputDevice->buttonNameMap + buttonsCount, alignof(const char*));
		inputDevice->buttonHashList = (StringId32*)Memory::getAlignedToTop(inputDevice->axisNameMap + axesCount, alignof(StringId32 ));
		inputDevice->axesHashList = (StringId32*)Memory::getAlignedToTop(inputDevice->buttonHashList + buttonsCount, alignof(StringId32 ));
		inputDevice->inputDeviceName = (char*)Memory::getAlignedToTop(inputDevice->axesHashList + axesCount, alignof(char));

		memset(inputDevice->lastStateList, 0, sizeof(uint8_t)*buttonsCount);
		memset(inputDevice->stateList, 0, sizeof(uint8_t)*buttonsCount);
		memset(inputDevice->axesList, 0, sizeof(Vector3)*axesCount);

		memcpy(inputDevice->buttonNameMap, buttonNameList, sizeof(const char*)*buttonsCount);
		memcpy(inputDevice->axisNameMap, axesNameList, sizeof(const char*)*axesCount);

		for (uint32_t i = 0; i < buttonsCount; ++i)
		{
			inputDevice->buttonHashList[i] = StringId32(buttonNameList[i]);
		}

		for (uint32_t i = 0; i < axesCount; ++i)
		{
			inputDevice->axesHashList[i] = StringId32(axesNameList[i]);
		}

		strcpy(inputDevice->inputDeviceName, name);

		return inputDevice;
	}

	void destroy(Allocator& a, InputDevice& id)
	{
		a.deallocate(&id);
	}

} // namespace InputDeviceFn

} // namespace Rio
