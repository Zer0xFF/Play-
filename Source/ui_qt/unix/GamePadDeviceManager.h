#pragma once

#include <atomic>
#include <thread>
#include <map>
#include <libevdev.h>

#include "GamePadUtils.h"
#include "GamePadInputEventInterface.h"
#include "Types.h"

#include "filesystem_def.h"

class CGamePadDeviceManager
{
public:
	typedef std::function<void(GamePadDeviceId, int, int, int, const input_absinfo*)> OnInputEvent;

	CGamePadDeviceManager(OnInputEvent);

	~CGamePadDeviceManager();

	struct inputdevice
	{
		std::string name;
		std::array<uint32, 6> uniq_id;
		std::string path;
		int fd = 0;
		struct libevdev* dev = nullptr;
	};
	typedef std::pair<std::string, CGamePadDeviceManager::inputdevice> inputdev_pair;
	OnInputEvent OnInputEventCallBack;

	void UpdateOnInputEventCallback(OnInputEvent);
	void DisconnectInputEventCallback();
	static bool OpenDevice(const fs::path&, inputdev_pair&);

private:
	std::map<std::string, CGamePadDeviceManager::inputdevice> m_devicelist;
	std::map<std::string, std::unique_ptr<CGamePadInputEventInterface>> m_GPIEList;
	std::atomic<bool> m_running;
	std::thread m_inputdevicelistenerthread;
	std::thread m_thread;
	std::map<std::string, CGamePadInputEventInterface::OnInputEventType::Connection> m_connectionlist;

	void UpdateDeviceList();
	void AddDevice(const fs::path&);
	void RemoveDevice(std::string);
	void InputDeviceListenerThread();
};
