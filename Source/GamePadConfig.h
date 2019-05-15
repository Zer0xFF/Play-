#pragma once

#include "Config.h"
#include "Singleton.h"

class CGamePadConfig : public Framework::CConfig
{
public:
	CGamePadConfig();
	virtual ~CGamePadConfig() = default;

	void SetConfigPath(CConfig::PathType path);
	static CConfig::PathType GetDefaultProfilePath();
	static CConfig::PathType GetProfilePath();
};
