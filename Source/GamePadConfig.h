#pragma once

#include "Config.h"
#include "Singleton.h"

#define DEFAULT_PROFILE ("default")

class CGamePadConfig : public Framework::CConfig
{
public:
	CGamePadConfig(const Framework::CConfig::PathType& path);
	virtual ~CGamePadConfig() = default;

	static CConfig::PathType GetProfilePath();
	static CConfig::PathType GetProfile(std::string = DEFAULT_PROFILE);
	static std::unique_ptr<CGamePadConfig> LoadProfile(std::string = DEFAULT_PROFILE);
};
