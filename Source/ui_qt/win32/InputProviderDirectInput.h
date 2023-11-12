#pragma once

#include "input/InputProvider.h"
#include "directinput/Manager.h"

class CInputProviderDirectInput : public CInputProvider
{
public:
	CInputProviderDirectInput();
	virtual ~CInputProviderDirectInput() = default;

	uint32 GetId() const override;
	std::string GetTargetDescription(const BINDINGTARGET&) const override;
	std::vector<DEVICEINFO> GetDevices() const override;
	void SetVibration(DeviceIdType deviceId, uint8 largeMotor, uint8 smallMotor) override;

	virtual bool InstanceAutoPadConfigure(int padIndex, DeviceIdType deviceId, CInputBindingManager* bindingManager) override;
	static void AutoPadConfigure(int padIndex, DeviceIdType deviceId, CInputBindingManager* bindingManager);

private:

	enum KEYID
	{
		KEYID_LTHUMB_X		= DIJOFS_X,
		KEYID_LTHUMB_Y		= DIJOFS_Y,
		KEYID_RTHUMB_X		= DIJOFS_Z,
		KEYID_RTHUMB_Y		= DIJOFS_RZ,
		KEYID_LTRIGGER		= DIJOFS_RX,
		KEYID_RTRIGGER		= DIJOFS_RY,
		KEYID_DPAD_UP		= DIJOFS_POV(0),
		KEYID_DPAD_RIGHT	= DIJOFS_POV(0),
		KEYID_DPAD_DOWN		= DIJOFS_POV(0),
		KEYID_DPAD_LEFT		= DIJOFS_POV(0),
		KEYID_START			= DIJOFS_BUTTON9,
		KEYID_BACK			= DIJOFS_BUTTON8,
		KEYID_LTHUMB		= DIJOFS_BUTTON10,
		KEYID_RTHUMB		= DIJOFS_BUTTON11,
		KEYID_LSHOULDER		= DIJOFS_BUTTON4,
		KEYID_RSHOULDER		= DIJOFS_BUTTON5,
		KEYID_A				= DIJOFS_BUTTON1,
		KEYID_B				= DIJOFS_BUTTON2,
		KEYID_X				= DIJOFS_BUTTON0,
		KEYID_Y				= DIJOFS_BUTTON3,
		KEYID_MAX
	};

	void HandleInputEvent(const GUID&, uint32, uint32);
	static BINDINGTARGET MakeBindingTarget(DeviceIdType deviceId, int keyCode, BINDINGTARGET::KEYTYPE keyType);

	std::unique_ptr<Framework::DirectInput::CManager> m_diManager;
};
