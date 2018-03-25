#pragma once

#include "Core/Thread/AtomicInt.h"
#include "Core/Types.h"

namespace Rio
{

struct OsEventType
{
	enum Enum
	{
		BUTTON,
		AXIS,
		STATUS,
		RESOLUTION,
		EXIT,
		PAUSE,
		RESUME,
		TEXT,
		NONE
	};
};

struct ButtonEvent
{
	uint16_t type = UINT16_MAX;
	uint16_t deviceId : 3;
	uint16_t deviceIndex : 2;
	uint16_t buttonIndex : 8;
	uint16_t isPressed : 1;
};

struct AxisEvent
{
	uint16_t type = UINT16_MAX;
	uint16_t deviceId : 3;
	uint16_t deviceIndex : 2;
	uint16_t axisIndex : 4;
	float axisX = 0.0f;
	float axisY = 0.0f;
	float axisZ = 0.0f;
};

struct StatusEvent
{
	uint16_t type = UINT16_MAX;
	uint16_t deviceId : 3;
	uint16_t deviceIndex : 2;
	uint16_t isConnected : 1;
};

struct ResolutionEvent
{
	uint16_t type = UINT16_MAX;
	uint16_t width = UINT16_MAX;
	uint16_t height = UINT16_MAX;
};

struct TextEvent
{
	uint16_t type = UINT16_MAX;
	uint8_t length = 0;
	uint8_t utf8[4];
};

union OsEvent
{
	OsEvent()
	{

	}

	uint16_t type = UINT16_MAX;
	ButtonEvent button;
	AxisEvent axis;
	StatusEvent status;
	ResolutionEvent resolution;
	TextEvent text;
};

// Single Producer Single Consumer event queue
// Used only to pass events from os thread to main thread
struct DeviceEventQueue
{
	AtomicInt tail{0};
	AtomicInt head{0};

	static const int32_t MAX_OS_EVENTS = 128;

	OsEvent osEventQueue[MAX_OS_EVENTS];

	DeviceEventQueue()
	{
	}

	void pushButtonEvent(uint16_t deviceId, uint16_t deviceIndex, uint16_t buttonId, bool isPressed)
	{
		OsEvent osEvent;
		osEvent.button.type = OsEventType::BUTTON;
		osEvent.button.deviceId = deviceId;
		osEvent.button.deviceIndex = deviceIndex;
		osEvent.button.buttonIndex = buttonId;
		osEvent.button.isPressed = isPressed;

		pushEvent(osEvent);
	}

	void pushAxisEvent(uint16_t deviceId, uint16_t deviceIndex, uint16_t axisId, float axisX, float axisY, float axisZ)
	{
		OsEvent osEvent;
		osEvent.axis.type = OsEventType::AXIS;
		osEvent.axis.deviceId = deviceId;
		osEvent.axis.deviceIndex = deviceIndex;
		osEvent.axis.axisIndex = axisId;
		osEvent.axis.axisX = axisX;
		osEvent.axis.axisY = axisY;
		osEvent.axis.axisZ = axisZ;

		pushEvent(osEvent);
	}

	void pushStatusEvent(uint16_t deviceId, uint16_t deviceIndex, bool isConnected)
	{
		OsEvent osEvent;
		osEvent.status.type = OsEventType::STATUS;
		osEvent.status.deviceId = deviceId;
		osEvent.status.deviceIndex = deviceIndex;
		osEvent.status.isConnected = isConnected;

		pushEvent(osEvent);
	}

	void pushResolutionEvent(uint16_t width, uint16_t height)
	{
		OsEvent osEvent;
		osEvent.resolution.type = OsEventType::RESOLUTION;
		osEvent.resolution.width = width;
		osEvent.resolution.height = height;

		pushEvent(osEvent);
	}

	void pushExitEvent()
	{
		OsEvent osEvent;
		osEvent.type = OsEventType::EXIT;

		pushEvent(osEvent);
	}

	void pushTextEvent(uint8_t length, uint8_t utf8[4])
	{
		OsEvent osEvent;
		osEvent.text.type = OsEventType::TEXT;
		osEvent.text.length = length;
		memcpy(osEvent.text.utf8, utf8, sizeof(osEvent.text.utf8));

		pushEvent(osEvent);
	}

	void pushNoneEvent()
	{
		OsEvent osEvent;
		osEvent.type = OsEventType::NONE;

		pushEvent(osEvent);
	}

	bool pushEvent(const OsEvent& osEvent)
	{
		const int32_t tail = this->tail.load();
		const int32_t tailNext = (tail + 1) % MAX_OS_EVENTS;

		if (tailNext != this->head.load())
		{
			osEventQueue[tail] = osEvent;
			this->tail.store(tailNext);
			return true;
		}

		return false;
	}

	bool popEvent(OsEvent& osEvent)
	{
		const int32_t head = this->head.load();

		if (head == this->tail.load())
		{
			return false;
		}

		osEvent = osEventQueue[head];
		this->head.store((head + 1) % MAX_OS_EVENTS);

		return true;
	}
};

} // namespace Rio
