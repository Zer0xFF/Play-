#include "AppConfig.h"
#include "GamePadConfig.h"
#include "PathUtils.h"

#define PROFILE_PATH (L"Profiles")


CGamePadConfig::CGamePadConfig()
	: CConfig(GetDefaultProfilePath())
{
}

Framework::CConfig::PathType CGamePadConfig::GetDefaultProfilePath()
{
	auto path = GetProfile();
	return path;
}

Framework::CConfig::PathType CGamePadConfig::GetProfilePath()
{
	auto profile_path = CAppConfig::GetBasePath() / PROFILE_PATH;
	Framework::PathUtils::EnsurePathExists(profile_path);
	return profile_path;
}

Framework::CConfig::PathType CGamePadConfig::GetProfile(std::string name)
{
	auto profile_path = CAppConfig::GetBasePath() / PROFILE_PATH;
	Framework::PathUtils::EnsurePathExists(profile_path);
	profile_path /= name;
	profile_path.replace_extension(".xml");
	return profile_path;
}

void CGamePadConfig::SetConfigPath(CConfig::PathType name)
{
	m_path = GetProfilePath() / name;
	m_path.replace_extension(".xml");
	m_preferences.clear();
	Load();
}
