#pragma once

#include <atomic>
#include "signal/Signal.h"
#include <thread>
#include <libevdev.h>
#include "Types.h"
#include "GamePadUtils.h"

class CGamePadInputEventInterface
{
public:
	CGamePadInputEventInterface(std::string);
	virtual ~CGamePadInputEventInterface();

	typedef Framework::CSignal<void(GamePadDeviceId, int, int, int, const input_absinfo*)> OnInputEventType;
	OnInputEventType OnInputEvent;

	GamePadDeviceId GetDeviceID();
private:
	GamePadDeviceId m_deviceId;
	std::string m_device;
	std::atomic<bool> m_running;
	std::thread m_thread;

	void InputDeviceListenerThread();
};
