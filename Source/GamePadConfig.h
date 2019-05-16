#pragma once

#include "Config.h"
#include "Singleton.h"

#define DEFAULT_PROFILE ("default")

class CGamePadConfig : public Framework::CConfig
{
public:
	CGamePadConfig();
	virtual ~CGamePadConfig() = default;

	void SetConfigPath(CConfig::PathType path);
	static CConfig::PathType GetDefaultProfilePath();
	static CConfig::PathType GetProfilePath();
	static CConfig::PathType GetProfile(std::string = DEFAULT_PROFILE);
};
