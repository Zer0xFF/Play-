#include "AppConfig.h"
#include "GamePadConfig.h"
#include "PathUtils.h"

#define PROFILE_PATH (L"profiles")

CGamePadConfig::CGamePadConfig(const Framework::CConfig::PathType& path)
	: CConfig(path)
{
}

std::unique_ptr<CGamePadConfig> CGamePadConfig::LoadProfile(std::string name)
{
	auto path = GetProfilePath() / name;
	path.replace_extension(".xml");
	return std::make_unique<CGamePadConfig>(path);
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
