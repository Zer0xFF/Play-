#include "InputProviderDirectInput.h"
#include "input/InputBindingManager.h"
#include "string_format.h"
#include "string_cast.h"

constexpr uint32 PROVIDER_ID = 'dinp';

static_assert(sizeof(GUID) <= sizeof(DeviceIdType::value_type) * DeviceIdTypeCount, "DeviceIdType cannot hold GUID");

static DeviceIdType GuidToDeviceId(const GUID& guid)
{
	DeviceIdType deviceId;
	for(auto& deviceIdElem : deviceId)
	{
		deviceIdElem = 0;
	}
	memcpy(deviceId.data(), &guid, sizeof(GUID));
	return deviceId;
}

GUID DeviceIdToGuid(const DeviceIdType& deviceId)
{
	GUID guid;
	memcpy(&guid, deviceId.data(), sizeof(GUID));
	return guid;
}

CInputProviderDirectInput::CInputProviderDirectInput()
{
	m_diManager = std::make_unique<Framework::DirectInput::CManager>(true);
	m_diManager->RegisterInputEventHandler(std::bind(&CInputProviderDirectInput::HandleInputEvent, this,
	                                                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_diManager->CreateJoysticks();
}

uint32 CInputProviderDirectInput::GetId() const
{
	return PROVIDER_ID;
}

std::string CInputProviderDirectInput::GetTargetDescription(const BINDINGTARGET& target) const
{
	auto deviceId = DeviceIdToGuid(target.deviceId);
	std::string deviceName = "Unknown Device";
	std::string deviceKeyName = "Unknown Key";

	DIDEVICEINSTANCE deviceInstance = {};
	if(m_diManager->GetDeviceInfo(deviceId, &deviceInstance))
	{
		deviceName = string_cast<std::string>(deviceInstance.tszInstanceName);
	}

	DIDEVICEOBJECTINSTANCE objectInstance = {};
	if(m_diManager->GetDeviceObjectInfo(deviceId, target.keyId, &objectInstance))
	{
		deviceKeyName = string_cast<std::string>(objectInstance.tszName);
	}

	return string_format("%s: %s", deviceName.c_str(), deviceKeyName.c_str());
}

std::vector<DEVICEINFO> CInputProviderDirectInput::GetDevices() const
{
	std::vector<DEVICEINFO> devices;
	for(auto deviceGuid : m_diManager->GetJoystickIds())
	{
		std::string deviceName = "Unknown Device";
		DIDEVICEINSTANCE deviceInstance = {};
		if(m_diManager->GetDeviceInfo(deviceGuid, &deviceInstance))
		{
			deviceName = string_cast<std::string>(deviceInstance.tszInstanceName);
		}
		auto deviceId = GuidToDeviceId(deviceGuid);
		devices.push_back({GetId(), deviceId, string_format("%s : 0x%X", deviceName.c_str(), deviceId)});
	}
	return devices;
}

void CInputProviderDirectInput::SetVibration(DeviceIdType deviceId, uint8 largeMotor, uint8 smallMotor)
{
	auto guid = DeviceIdToGuid(deviceId);
	m_diManager->SetJoystickVibration(guid, largeMotor, smallMotor);
}

void CInputProviderDirectInput::HandleInputEvent(const GUID& deviceId, uint32 keyId, uint32 value)
{
	DIDEVICEOBJECTINSTANCE objectInstance = {};
	if(!m_diManager->GetDeviceObjectInfo(deviceId, keyId, &objectInstance))
	{
		return;
	}

	BINDINGTARGET tgt;
	tgt.providerId = PROVIDER_ID;
	tgt.deviceId = GuidToDeviceId(deviceId);
	tgt.keyId = keyId;

	uint32 modifiedValue = value;
	if(objectInstance.dwType & DIDFT_AXIS)
	{
		tgt.keyType = BINDINGTARGET::KEYTYPE::AXIS;
		modifiedValue = value >> 8;
	}
	else if(objectInstance.dwType & DIDFT_BUTTON)
	{
		tgt.keyType = BINDINGTARGET::KEYTYPE::BUTTON;
	}
	else if(objectInstance.dwType & DIDFT_POV)
	{
		tgt.keyType = BINDINGTARGET::KEYTYPE::POVHAT;
		if(value == -1)
		{
			modifiedValue = BINDINGTARGET::POVHAT_MAX;
		}
		else
		{
			modifiedValue = value / 4500;
		}
	}
	else
	{
		return;
	}

	OnInput(tgt, modifiedValue);
}

bool CInputProviderDirectInput::InstanceAutoPadConfigure(int padIndex, DeviceIdType deviceId, CInputBindingManager* bindingManager)
{
	AutoPadConfigure(padIndex, deviceId, bindingManager);
	return true;
}

BINDINGTARGET CInputProviderDirectInput::MakeBindingTarget(DeviceIdType deviceId, int keyCode, BINDINGTARGET::KEYTYPE keyType)
{
	return BINDINGTARGET(PROVIDER_ID, deviceId, keyCode, keyType);
}

void CInputProviderDirectInput::AutoPadConfigure(int padIndex, DeviceIdType deviceId, CInputBindingManager* bindingManager)
{


#define SBind(PS2BTN, KEY)                                                                                                                                 \
	{                                                                                                                                                      \
		bindingManager->SetSimpleBinding(padIndex, PS2::CControllerInfo::PS2BTN, MakeBindingTarget(deviceId, KEYID::KEY, BINDINGTARGET::KEYTYPE::BUTTON)); \
	}

#define SAxisBind(PS2BTN, KEY)                                                                                                                           \
	{                                                                                                                                                    \
		bindingManager->SetSimpleBinding(padIndex, PS2::CControllerInfo::PS2BTN, MakeBindingTarget(deviceId, KEYID::KEY, BINDINGTARGET::KEYTYPE::AXIS)); \
	}

#define SPovHatBind(PS2BTN, KEY, povHatValue)                                                                                                                           \
	{                                                                                                                                                                   \
		bindingManager->SetPovHatBinding(padIndex, PS2::CControllerInfo::PS2BTN, MakeBindingTarget(deviceId, KEYID::KEY, BINDINGTARGET::KEYTYPE::POVHAT), povHatValue); \
	}

	SBind(START, KEYID_START);
	SBind(SELECT, KEYID_BACK);

	SPovHatBind(DPAD_UP, KEYID_DPAD_UP, 0);
	SPovHatBind(DPAD_LEFT, KEYID_DPAD_RIGHT, 2);
	SPovHatBind(DPAD_RIGHT, KEYID_DPAD_DOWN, 6);
	SPovHatBind(DPAD_DOWN, KEYID_DPAD_LEFT, 4);

	SBind(L1, KEYID_LSHOULDER);
	SBind(L2, KEYID_LTRIGGER);
	SBind(L3, KEYID_LTHUMB);
	SBind(R1, KEYID_RSHOULDER);
	SBind(R2, KEYID_RTRIGGER);
	SBind(R3, KEYID_RTHUMB);

	SBind(TRIANGLE, KEYID_Y);
	SBind(CIRCLE, KEYID_B);
	SBind(CROSS, KEYID_A);
	SBind(SQUARE, KEYID_X);

	SAxisBind(ANALOG_LEFT_X, KEYID_LTHUMB_X);
	SAxisBind(ANALOG_LEFT_Y, KEYID_LTHUMB_Y);
	SAxisBind(ANALOG_RIGHT_X, KEYID_RTHUMB_X);
	SAxisBind(ANALOG_RIGHT_Y, KEYID_RTHUMB_Y);

	auto targetBinding = MakeBindingTarget(deviceId, -1, BINDINGTARGET::KEYTYPE::MOTOR);
	bindingManager->SetMotorBinding(padIndex, targetBinding);

#undef SBind
#undef SAxisBind
#undef SPovHatBind
}
