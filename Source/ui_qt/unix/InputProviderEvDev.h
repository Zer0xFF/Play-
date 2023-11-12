#pragma once

#include "input/InputProvider.h"
#include "GamePadUtils.h"
#include "GamePadDeviceManager.h"

class CInputProviderEvDev : public CInputProvider
{
public:
	CInputProviderEvDev();
	virtual ~CInputProviderEvDev() = default;

	uint32 GetId() const override;
	std::string GetTargetDescription(const BINDINGTARGET&) const override;
	void SetVibration(DeviceIdType deviceId, uint8 largeMotor, uint8 smallMotor) override;

private:
	void OnEvDevInputEvent(GamePadDeviceId, int, int, int, const input_absinfo*);
	CGamePadDeviceManager m_GPDM;
};
