#include "AppConfig.h"
#include "GamePadConfig.h"
#include "PathUtils.h"

#define PROFILE_PATH (L"Profiles")
#define DEFAULT_PROFILE (L"default.xml")


CGamePadConfig::CGamePadConfig()
	: CConfig(GetDefaultProfilePath())
{
}

Framework::CConfig::PathType CGamePadConfig::GetDefaultProfilePath()
{
	auto path = GetProfilePath() / DEFAULT_PROFILE;
	return path;
}

Framework::CConfig::PathType CGamePadConfig::GetProfilePath()
{
	auto profile_path = CAppConfig::GetBasePath() / PROFILE_PATH;
	Framework::PathUtils::EnsurePathExists(profile_path);
	return profile_path;
}

void CGamePadConfig::SetConfigPath(CConfig::PathType name)
{
	m_path = GetProfilePath() / name;
	m_path.replace_extension(".xml");
	m_preferences.clear();
	Load();
}
