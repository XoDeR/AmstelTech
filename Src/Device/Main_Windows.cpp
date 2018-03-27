#include "Config.h"

#if RIO_PLATFORM_WINDOWS

#include "Core/CommandLine.h"
#include "Core/Thread/Thread.h"

#include "Device/Device.h"
#include "Device/DeviceEventQueue.h"

#if AMSTEL_ENGINE_RESOURCE_MANAGER
#include "Resource/DataCompiler.h"
#endif // AMSTEL_ENGINE_RESOURCE_MANAGER

#include "RioRenderer/Platform.h"

#include <winsock2.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windowsx.h>
#include <xinput.h>

namespace Rio
{

static KeyboardButton::Enum getKeyboardButtonTypeByWindowsKeyId(int32_t windowsKeyId)
{
	switch (windowsKeyId)
	{
	case VK_BACK: return KeyboardButton::BACKSPACE;
	case VK_TAB: return KeyboardButton::TAB;
	case VK_SPACE: return KeyboardButton::SPACE;
	case VK_ESCAPE: return KeyboardButton::ESCAPE;
	case VK_RETURN: return KeyboardButton::ENTER;
	case VK_F1: return KeyboardButton::F1;
	case VK_F2: return KeyboardButton::F2;
	case VK_F3: return KeyboardButton::F3;
	case VK_F4: return KeyboardButton::F4;
	case VK_F5: return KeyboardButton::F5;
	case VK_F6: return KeyboardButton::F6;
	case VK_F7: return KeyboardButton::F7;
	case VK_F8: return KeyboardButton::F8;
	case VK_F9: return KeyboardButton::F9;
	case VK_F10: return KeyboardButton::F10;
	case VK_F11: return KeyboardButton::F11;
	case VK_F12: return KeyboardButton::F12;
	case VK_HOME: return KeyboardButton::HOME;
	case VK_LEFT: return KeyboardButton::LEFT;
	case VK_UP: return KeyboardButton::UP;
	case VK_RIGHT: return KeyboardButton::RIGHT;
	case VK_DOWN: return KeyboardButton::DOWN;
	case VK_PRIOR: return KeyboardButton::PAGE_UP;
	case VK_NEXT: return KeyboardButton::PAGE_DOWN;
	case VK_INSERT: return KeyboardButton::INS;
	case VK_DELETE: return KeyboardButton::DEL;
	case VK_END: return KeyboardButton::END;
	case VK_LSHIFT: return KeyboardButton::SHIFT_LEFT;
	case VK_RSHIFT: return KeyboardButton::SHIFT_RIGHT;
	case VK_LCONTROL: return KeyboardButton::CTRL_LEFT;
	case VK_RCONTROL: return KeyboardButton::CTRL_RIGHT;
	case VK_CAPITAL: return KeyboardButton::CAPS_LOCK;
	case VK_LMENU: return KeyboardButton::ALT_LEFT;
	case VK_RMENU: return KeyboardButton::ALT_RIGHT;
	case VK_LWIN: return KeyboardButton::SUPER_LEFT;
	case VK_RWIN: return KeyboardButton::SUPER_RIGHT;
	case VK_NUMLOCK: return KeyboardButton::NUM_LOCK;
	case VK_DECIMAL: return KeyboardButton::NUMPAD_DELETE;
	case VK_MULTIPLY: return KeyboardButton::NUMPAD_MULTIPLY;
	case VK_ADD: return KeyboardButton::NUMPAD_ADD;
	case VK_SUBTRACT: return KeyboardButton::NUMPAD_SUBTRACT;
	case VK_DIVIDE: return KeyboardButton::NUMPAD_DIVIDE;
	case VK_NUMPAD0: return KeyboardButton::NUMPAD_0;
	case VK_NUMPAD1: return KeyboardButton::NUMPAD_1;
	case VK_NUMPAD2: return KeyboardButton::NUMPAD_2;
	case VK_NUMPAD3: return KeyboardButton::NUMPAD_3;
	case VK_NUMPAD4: return KeyboardButton::NUMPAD_4;
	case VK_NUMPAD5: return KeyboardButton::NUMPAD_5;
	case VK_NUMPAD6: return KeyboardButton::NUMPAD_6;
	case VK_NUMPAD7: return KeyboardButton::NUMPAD_7;
	case VK_NUMPAD8: return KeyboardButton::NUMPAD_8;
	case VK_NUMPAD9: return KeyboardButton::NUMPAD_9;
	case '0': return KeyboardButton::NUMBER_0;
	case '1': return KeyboardButton::NUMBER_1;
	case '2': return KeyboardButton::NUMBER_2;
	case '3': return KeyboardButton::NUMBER_3;
	case '4': return KeyboardButton::NUMBER_4;
	case '5': return KeyboardButton::NUMBER_5;
	case '6': return KeyboardButton::NUMBER_6;
	case '7': return KeyboardButton::NUMBER_7;
	case '8': return KeyboardButton::NUMBER_8;
	case '9': return KeyboardButton::NUMBER_9;
	case 'A': return KeyboardButton::A;
	case 'B': return KeyboardButton::B;
	case 'C': return KeyboardButton::C;
	case 'D': return KeyboardButton::D;
	case 'E': return KeyboardButton::E;
	case 'F': return KeyboardButton::F;
	case 'G': return KeyboardButton::G;
	case 'H': return KeyboardButton::H;
	case 'I': return KeyboardButton::I;
	case 'J': return KeyboardButton::J;
	case 'K': return KeyboardButton::K;
	case 'L': return KeyboardButton::L;
	case 'M': return KeyboardButton::M;
	case 'N': return KeyboardButton::N;
	case 'O': return KeyboardButton::O;
	case 'P': return KeyboardButton::P;
	case 'Q': return KeyboardButton::Q;
	case 'R': return KeyboardButton::R;
	case 'S': return KeyboardButton::S;
	case 'T': return KeyboardButton::T;
	case 'U': return KeyboardButton::U;
	case 'V': return KeyboardButton::V;
	case 'W': return KeyboardButton::W;
	case 'X': return KeyboardButton::X;
	case 'Y': return KeyboardButton::Y;
	case 'Z': return KeyboardButton::Z;
	default: return KeyboardButton::COUNT;
	}
}

struct XInputKeyToJoypadButton
{
	WORD bit = 0;
	JoypadButton::Enum button = JoypadButton::COUNT;
};

static XInputKeyToJoypadButton XInputKeyToJoypadButtonMapStatic[] =
{
	{ XINPUT_GAMEPAD_DPAD_UP, JoypadButton::UP },
	{ XINPUT_GAMEPAD_DPAD_DOWN, JoypadButton::DOWN },
	{ XINPUT_GAMEPAD_DPAD_LEFT, JoypadButton::LEFT },
	{ XINPUT_GAMEPAD_DPAD_RIGHT, JoypadButton::RIGHT },
	{ XINPUT_GAMEPAD_START, JoypadButton::START },
	{ XINPUT_GAMEPAD_BACK, JoypadButton::BACK },
	{ XINPUT_GAMEPAD_LEFT_THUMB, JoypadButton::THUMB_LEFT },
	{ XINPUT_GAMEPAD_RIGHT_THUMB, JoypadButton::THUMB_RIGHT },
	{ XINPUT_GAMEPAD_LEFT_SHOULDER, JoypadButton::SHOULDER_LEFT },
	{ XINPUT_GAMEPAD_RIGHT_SHOULDER, JoypadButton::SHOULDER_RIGHT },
	{ XINPUT_GAMEPAD_A, JoypadButton::A },
	{ XINPUT_GAMEPAD_B, JoypadButton::B },
	{ XINPUT_GAMEPAD_X, JoypadButton::X },
	{ XINPUT_GAMEPAD_Y, JoypadButton::Y }
};

struct Joypad
{
	struct Axis
	{
		float leftX = 0.0f;
		float leftY = 0.0f;
		float leftZ = 0.0f;

		float rightX = 0.0f;
		float rightY = 0.0f;
		float rightZ = 0.0f;
	};

	XINPUT_STATE joypadInputState[AMSTEL_ENGINE_MAX_JOYPADS];
	Axis joypadAxisList[AMSTEL_ENGINE_MAX_JOYPADS];
	bool isJoypadConnectedList[AMSTEL_ENGINE_MAX_JOYPADS];

	void init()
	{
		memset(&joypadInputState, 0, sizeof(joypadInputState));
		memset(&joypadAxisList, 0, sizeof(joypadAxisList));
		memset(&isJoypadConnectedList, 0, sizeof(isJoypadConnectedList));
	}

	void update(DeviceEventQueue& deviceEventQueue)
	{
		for (uint8_t i = 0; i < AMSTEL_ENGINE_MAX_JOYPADS; ++i)
		{
			XINPUT_STATE state;
			memset(&state, 0, sizeof(state));

			const DWORD result = XInputGetState(i, &state);
			const bool connected = result == ERROR_SUCCESS;

			if (connected != isJoypadConnectedList[i])
			{
				deviceEventQueue.pushStatusEvent(InputDeviceType::JOYPAD, i, connected);
			}

			isJoypadConnectedList[i] = connected;

			if (!connected || state.dwPacketNumber == joypadInputState[i].dwPacketNumber)
			{
				continue;
			}

			XINPUT_GAMEPAD& gamepad = joypadInputState[i].Gamepad;

			const WORD difference = state.Gamepad.wButtons ^ gamepad.wButtons;
			const WORD current = state.Gamepad.wButtons;
			if (difference != 0)
			{
				for (uint8_t buttonId = 0; buttonId < countof(XInputKeyToJoypadButtonMapStatic); ++buttonId)
				{
					WORD bit = XInputKeyToJoypadButtonMapStatic[buttonId].bit;
					if (bit & difference)
					{
						deviceEventQueue.pushButtonEvent(InputDeviceType::JOYPAD
							, i
							, XInputKeyToJoypadButtonMapStatic[buttonId].button
							, (current & bit) != 0
							);
						gamepad.wButtons = current;
					}
				}
			}

			if (state.Gamepad.sThumbLX != gamepad.sThumbLX)
			{
				SHORT value = state.Gamepad.sThumbLX;
				value = value > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || value < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? value : 0;

				joypadAxisList[0].leftX = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) / float(INT16_MAX - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
					: 0.0f
					;

				deviceEventQueue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::LEFT
					, joypadAxisList[0].leftX
					, joypadAxisList[0].leftY
					, joypadAxisList[0].leftZ
					);

				gamepad.sThumbLX = state.Gamepad.sThumbLX;
			}

			if (state.Gamepad.sThumbLY != gamepad.sThumbLY)
			{
				SHORT value = state.Gamepad.sThumbLY;
				value = value > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || value < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
					? value : 0;

				joypadAxisList[0].leftY = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) / float(INT16_MAX - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
					: 0.0f
					;

				deviceEventQueue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::LEFT
					, joypadAxisList[0].leftX
					, joypadAxisList[0].leftY
					, joypadAxisList[0].leftZ
					);

				gamepad.sThumbLY = state.Gamepad.sThumbLY;
			}

			if (state.Gamepad.bLeftTrigger != gamepad.bLeftTrigger)
			{
				BYTE value = state.Gamepad.bLeftTrigger;
				value = value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? value : 0;

				joypadAxisList[0].leftZ = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_TRIGGER_THRESHOLD : -XINPUT_GAMEPAD_TRIGGER_THRESHOLD)) / float(UINT8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
					: 0.0f
					;
				deviceEventQueue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::LEFT
					, joypadAxisList[0].leftX
					, joypadAxisList[0].leftY
					, joypadAxisList[0].leftZ
					);

				gamepad.bLeftTrigger = state.Gamepad.bLeftTrigger;
			}

			if (state.Gamepad.sThumbRX != gamepad.sThumbRX)
			{
				SHORT value = state.Gamepad.sThumbRX;
				value = value > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || value < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE	? value : 0;

				joypadAxisList[0].rightX = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE : -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) / float(INT16_MAX - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
					: 0.0f
					;

				deviceEventQueue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::RIGHT
					, joypadAxisList[0].rightX
					, joypadAxisList[0].rightY
					, joypadAxisList[0].rightZ
					);

				gamepad.sThumbRX = state.Gamepad.sThumbRX;
			}

			if (state.Gamepad.sThumbRY != gamepad.sThumbRY)
			{
				SHORT value = state.Gamepad.sThumbRY;
				value = value > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || value < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
					? value : 0;

				joypadAxisList[0].rightY = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE : -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) / float(INT16_MAX - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
					: 0.0f
					;

				deviceEventQueue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::RIGHT
					, joypadAxisList[0].rightX
					, joypadAxisList[0].rightY
					, joypadAxisList[0].rightZ
					);

				gamepad.sThumbRY = state.Gamepad.sThumbRY;
			}

			if (state.Gamepad.bRightTrigger != gamepad.bRightTrigger)
			{
				BYTE value = state.Gamepad.bRightTrigger;
				value = value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? value : 0;

				joypadAxisList[0].rightZ = value != 0
					? float(value + (value < 0 ? XINPUT_GAMEPAD_TRIGGER_THRESHOLD : -XINPUT_GAMEPAD_TRIGGER_THRESHOLD)) / float(UINT8_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
					: 0.0f
					;

				deviceEventQueue.pushAxisEvent(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::RIGHT
					, joypadAxisList[0].rightX
					, joypadAxisList[0].rightY
					, joypadAxisList[0].rightZ
					);

				gamepad.bRightTrigger = state.Gamepad.bRightTrigger;
			}
		}
	}
};

static bool doExitApplicationStatic = false;

struct MainThreadArguments
{
	DeviceOptions* deviceOptions = nullptr;
};

int32_t mainThreadFunction(void* userData)
{
	MainThreadArguments* mainThreadArguments = (MainThreadArguments*)userData;
	Rio::run(*mainThreadArguments->deviceOptions);
	doExitApplicationStatic = true;
	return EXIT_SUCCESS;
}

struct WindowsDevice
{
	HWND windowsWindowHandle = NULL;
	DeviceEventQueue deviceEventQueue;
	Joypad joypad;

	WindowsDevice()
	{
	}

	int32_t	run(DeviceOptions* deviceOptions)
	{
		MainThreadArguments mainThreadArguments;
		mainThreadArguments.deviceOptions = deviceOptions;

		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
		WNDCLASSEX windowClass;
		memset(&windowClass, 0, sizeof(windowClass));
		windowClass.cbSize = sizeof(windowClass);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = windowProcedure;
		windowClass.hInstance = instance;
		windowClass.hIcon = LoadIcon(instance, IDI_APPLICATION);
		windowClass.hCursor = LoadCursor(instance, IDC_ARROW);
		windowClass.lpszClassName = "Rio";
		windowClass.hIconSm = LoadIcon(instance, IDI_APPLICATION);
		RegisterClassExA(&windowClass);

		windowsWindowHandle = CreateWindowA("Rio"
			, "Rio"
			, WS_OVERLAPPEDWINDOW | WS_VISIBLE
			, deviceOptions->windowX
			, deviceOptions->windowY
			, deviceOptions->windowWidth
			, deviceOptions->windowHeight
			, 0
			, NULL
			, instance
			, 0
		);

		RIO_ASSERT(windowsWindowHandle != NULL, "CreateWindowA: GetLastError = %d", GetLastError());

		Thread mainThread;
		mainThread.start(mainThreadFunction, &mainThreadArguments);

		MSG windowsMessage;
		windowsMessage.message = WM_NULL;

		while (!doExitApplicationStatic)
		{
			joypad.update(deviceEventQueue);

			while (PeekMessage(&windowsMessage, NULL, 0U, 0U, PM_REMOVE) != 0)
			{
				TranslateMessage(&windowsMessage);
				DispatchMessage(&windowsMessage);
			}
		}

		mainThread.stop();
		DestroyWindow(windowsWindowHandle);

		return EXIT_SUCCESS;
	}

	LRESULT pumpWindowsEvents(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
	{
		switch (id)
		{
		case WM_DESTROY:
			break;

		case WM_QUIT:
		case WM_CLOSE:
			doExitApplicationStatic = true;
			deviceEventQueue.pushExitEvent();
			return 0;

		case WM_SIZE:
			{
				uint32_t width  = GET_X_LPARAM(lparam);
				uint32_t height = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushResolutionEvent(width, height);
			}
			break;

		case WM_SYSCOMMAND:
			switch (wparam)
			{
			case SC_MINIMIZE:
			case SC_RESTORE:
				{
					HWND parent = GetWindow(hwnd, GW_OWNER);
					if (parent != NULL)
					{
						PostMessage(parent, id, wparam, lparam);
					}
				}
				break;
			}
			break;

		case WM_MOUSEWHEEL:
			{
				int32_t mouseX = GET_X_LPARAM(lparam);
				int32_t mouseY = GET_Y_LPARAM(lparam);
				short delta = GET_WHEEL_DELTA_WPARAM(wparam);
				deviceEventQueue.pushAxisEvent(InputDeviceType::MOUSE
					, 0
					, MouseAxis::WHEEL
					, 0.0f
					, (float)(delta/WHEEL_DELTA)
					, 0.0f
					);
			}
			break;

		case WM_MOUSEMOVE:
			{
				int32_t mouseX = GET_X_LPARAM(lparam);
				int32_t mouseY = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushAxisEvent(InputDeviceType::MOUSE
					, 0
					, MouseAxis::CURSOR
					, (float)mouseX
					, (float)mouseY
					, 0.0f
					);
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			{
				int32_t mouseX = GET_X_LPARAM(lparam);
				int32_t mouseY = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushButtonEvent(InputDeviceType::MOUSE
					, 0
					, MouseButton::LEFT
					, id == WM_LBUTTONDOWN
					);
			}
			break;

		case WM_RBUTTONUP:
		case WM_RBUTTONDOWN:
			{
				int32_t mouseX = GET_X_LPARAM(lparam);
				int32_t mouseY = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushButtonEvent(InputDeviceType::MOUSE
					, 0
					, MouseButton::RIGHT
					, id == WM_RBUTTONDOWN
					);
			}
			break;

		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			{
				int32_t mouseX = GET_X_LPARAM(lparam);
				int32_t mouseY = GET_Y_LPARAM(lparam);
				deviceEventQueue.pushButtonEvent(InputDeviceType::MOUSE
					, 0
					, MouseButton::MIDDLE
					, id == WM_MBUTTONDOWN
					);
			}
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
			{
				KeyboardButton::Enum keyboardButton = getKeyboardButtonTypeByWindowsKeyId(wparam & 0xff);

				if (keyboardButton != KeyboardButton::COUNT)
				{
					deviceEventQueue.pushButtonEvent(InputDeviceType::KEYBOARD
						, 0
						, keyboardButton
						, (id == WM_KEYDOWN || id == WM_SYSKEYDOWN)
						);
				}
			}
			break;

		case WM_CHAR:
			{
				uint8_t utf8[4] = { 0 };
				uint8_t length = (uint8_t)WideCharToMultiByte(CP_UTF8
					, 0
					, (LPCWSTR)&wparam
					, 1
					, (LPSTR)utf8
					, sizeof(utf8)
					, NULL
					, NULL
					);

				if (length != 0)
				{
					deviceEventQueue.pushTextEvent(length, utf8);
				}
			}
			break;

		default:
			break;
		}

		return DefWindowProc(hwnd, id, wparam, lparam);
	}

	static LRESULT CALLBACK windowProcedure(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);
};

static WindowsDevice windowsDeviceStatic;

LRESULT CALLBACK WindowsDevice::windowProcedure(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
	return windowsDeviceStatic.pumpWindowsEvents(hwnd, id, wparam, lparam);
}

struct WindowWindows : public Window
{
	HWND windowsWindowHandle = NULL;
	uint16_t windowPositionX = 0;
	uint16_t windowPositionY = 0;
	uint16_t windowWidth = RIO_DEFAULT_WINDOW_WIDTH;
	uint16_t windowHeight = RIO_DEFAULT_WINDOW_HEIGHT;

	WindowWindows()
	{
		windowsWindowHandle = windowsDeviceStatic.windowsWindowHandle;
	}

	void open(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		this->windowPositionX = x;
		this->windowPositionY = y;
		this->windowWidth = width;
		this->windowHeight = height;
	}

	void close()
	{
	}

	void rioRendererSetup()
	{
		RioRenderer::PlatformData platformData;
		memset(&platformData, 0, sizeof(platformData));
		platformData.nativeWindowHandle = this->windowsWindowHandle;
		RioRenderer::setPlatformData(platformData);
	}

	void show()
	{
		ShowWindow(windowsWindowHandle, SW_SHOW);
	}

	void hide()
	{
		ShowWindow(windowsWindowHandle, SW_HIDE);
	}

	void resize(uint16_t width, uint16_t height)
	{
		this->windowWidth = width;
		this->windowHeight = height;
		MoveWindow(windowsWindowHandle, this->windowPositionX, this->windowPositionY, width, height, FALSE);
	}

	void move(uint16_t x, uint16_t y)
	{
		this->windowPositionX = x;
		this->windowPositionY = y;
		MoveWindow(windowsWindowHandle, x, y, this->windowWidth, this->windowHeight, FALSE);
	}

	void minimize()
	{
		ShowWindow(windowsWindowHandle, SW_MINIMIZE);
	}

	void restore()
	{
		ShowWindow(windowsWindowHandle, SW_RESTORE);
	}

	const char* getWindowTitle()
	{
		static char buffer[512];
		memset(buffer, 0, sizeof(buffer));
		GetWindowText(windowsWindowHandle, buffer, sizeof(buffer));
		return buffer;
	}

	void setWindowTitle(const char* title)
	{
		SetWindowText(windowsWindowHandle, title);
	}

	void setShowCursor(bool show)
	{
		ShowCursor(show);
	}

	void setIsFullscreen(bool /*fullscreen*/)
	{
		// Not implemented yet
	}

	void* getWindowHandle()
	{
		return (void*)(uintptr_t)windowsWindowHandle;
	}
};

namespace WindowFn
{
	Window* create(Allocator& a)
	{
		return RIO_NEW(a, WindowWindows)();
	}

	void destroy(Allocator& a, Window& window)
	{
		RIO_DELETE(a, &window);
	}

} // namespace WindowFn

struct DisplayWindows : public Display
{
	void getDisplayModeList(Array<DisplayMode>& /*modes*/)
	{
		// Not implemented yet
	}

	void setMode(uint32_t /*id*/)
	{
		// Not implemented yet
	}
};

namespace DisplayFn
{
	Display* create(Allocator& a)
	{
		return RIO_NEW(a, DisplayWindows)();
	}

	void destroy(Allocator& a, Display& d)
	{
		RIO_DELETE(a, &d);
	}

} // namespace DisplayFn

bool getNextOsEvent(OsEvent& osEvent)
{
	return windowsDeviceStatic.deviceEventQueue.popEvent(osEvent);
}

} // namespace Rio

struct InitMemoryGlobals
{
	InitMemoryGlobals()
	{
		Rio::MemoryGlobalFn::init();
	}

	~InitMemoryGlobals()
	{
		Rio::MemoryGlobalFn::shutdown();
	}
};

int main(int argumentsCount, char** argumentList)
{
	using namespace Rio;

	WSADATA wsaData;
	int windowsSocketsError = WSAStartup(MAKEWORD(2, 2), &wsaData);
	RIO_ASSERT(windowsSocketsError == 0, "WSAStartup: error = %d", windowsSocketsError);
	RIO_UNUSED(wsaData);
	RIO_UNUSED(windowsSocketsError);

	CommandLine commandLine(argumentsCount, (const char**)argumentList);

#if AMSTEL_ENGINE_RESOURCE_MANAGER
	if (commandLine.hasOption("compile") || commandLine.hasOption("serverMode"))
	{
		// Run data compiler
		if (dataCompilerMain(argc, argv) != EXIT_SUCCESS || !commandLine.hasOption("continueAfterCompilation"))
		{
			return EXIT_FAILURE;
		}
	}
#endif // AMSTEL_ENGINE_RESOURCE_MANAGER

	// Run engine
	InitMemoryGlobals initMemoryGlobals;
	RIO_UNUSED(initMemoryGlobals);

	DeviceOptions deviceOptions(getDefaultAllocator(), argumentsCount, (const char**)argumentList);
	int result = deviceOptions.parse();

	if (result == EXIT_SUCCESS)
	{
		result = windowsDeviceStatic.run(&deviceOptions);
	}

	WSACleanup();

	return result;
}

#endif // RIO_PLATFORM_WINDOWS
