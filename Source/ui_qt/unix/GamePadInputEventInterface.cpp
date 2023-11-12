#include "GamePadInputEventInterface.h"
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <csignal>
#include <cstring>
#include <unistd.h>

CGamePadInputEventInterface::CGamePadInputEventInterface(int fd, struct libevdev* dev, GamePadDeviceId deviceId)
    : m_fd(fd)
    , m_dev(dev)
    , m_deviceId(deviceId)
    , m_running(true)
{
	m_thread = std::thread([this]() { InputDeviceListenerThread(); });
}

CGamePadInputEventInterface::~CGamePadInputEventInterface()
{
	m_running = false;
	OnInputEvent.Reset();
	m_thread.join();
}

void CGamePadInputEventInterface::InputDeviceListenerThread()
{
	struct timespec ts;
	ts.tv_nsec = 5e+8; // 500 millisecond

	fd_set fds;

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	while(m_running)
	{
		FD_ZERO(&fds);
		FD_SET(m_fd, &fds);
		if(pselect(m_fd + 1, &fds, NULL, NULL, &ts, &mask) == 0) continue;

		int rc = 0;
		do
		{
			struct input_event ev;
			rc = libevdev_next_event(m_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
			if(rc == LIBEVDEV_READ_STATUS_SYNC)
			{
				while(rc == LIBEVDEV_READ_STATUS_SYNC)
				{
					rc = libevdev_next_event(m_dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
				}
			}
			else if(rc == LIBEVDEV_READ_STATUS_SUCCESS && ev.type != EV_SYN)
			{
				const struct input_absinfo* abs = nullptr;
				if(ev.type == EV_ABS) abs = libevdev_get_abs_info(m_dev, ev.code);
				OnInputEvent(m_deviceId, ev.code, ev.value, ev.type, abs);
			}
		} while(rc != -EAGAIN && m_running);
	}
}
