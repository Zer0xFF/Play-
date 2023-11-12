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
	CGamePadInputEventInterface(int fd, struct libevdev*, GamePadDeviceId deviceId);
	virtual ~CGamePadInputEventInterface();

	void SetVibration(uint8 largeMotor, uint8 smallMotor);
	typedef Framework::CSignal<void(GamePadDeviceId, int, int, int, const input_absinfo*)> OnInputEventType;
	OnInputEventType OnInputEvent;

private:
	GamePadDeviceId m_deviceId;
	std::string m_device;
	std::atomic<bool> m_running;
	std::thread m_thread;

	int m_fd = 0;
	struct libevdev* m_dev = nullptr;

	void InputDeviceListenerThread();
};
