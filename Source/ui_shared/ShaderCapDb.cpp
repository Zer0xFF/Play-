#include <cassert>
#include "../AppConfig.h"
#include "string_format.h"
#include "PathUtils.h"
#include "ShaderCapDb.h"
#include "sqlite/SqliteStatement.h"

using namespace ShaderCapDb;

#define DATABASE_VERSION 2

static const char* g_dbFileName = "shaderCap.db";

static const char* g_shaderCacheTableCreateStatement =
    "CREATE TABLE IF NOT EXISTS shaderCap"
    "("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    discId VARCHAR(10) DEFAULT '',"
    "    shaderCap INTEGER,"
    "    UNIQUE(discId, shaderCap)"
    ")";

CClient::CClient()
{
	m_dbPath = CAppConfig::GetInstance().GetBasePath() / g_dbFileName;

	CheckDbVersion();

	m_db = Framework::CSqliteDb(Framework::PathUtils::GetNativeStringFromPath(m_dbPath).c_str(),
	                            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

	{
		auto query = string_format("PRAGMA user_version = %d", DATABASE_VERSION);
		Framework::CSqliteStatement statement(m_db, query.c_str());
		statement.StepNoResult();
	}

	{
		Framework::CSqliteStatement statement(m_db, g_shaderCacheTableCreateStatement);
		statement.StepNoResult();
	}
}

std::vector<ShaderCap> CClient::GetShaderCaps(const char* discId)
{
	std::string query = "SELECT shaderCap FROM shaderCap WHERE discId = ?";

	std::vector<ShaderCap> shaderCap;

	Framework::CSqliteStatement statement(m_db, query.c_str());
	statement.BindText(1, discId, true);
	while(statement.Step())
	{
		auto shaderInfo = ReadShaderCap(statement);
		shaderCap.push_back(shaderInfo);
	}

	return shaderCap;
}

void CClient::RegisterShaderCap(const char* discId, uint32_t shaderCap)
{
	Framework::CSqliteStatement statement(m_db, "INSERT OR IGNORE INTO shaderCap (shaderCap, discId) VALUES (?,?)");
	sqlite3_bind_int64(statement, 1, shaderCap);
	statement.BindText(2, discId, true);
	statement.StepNoResult();
}

void CClient::UnregisterShaderCap(const char* discId)
{
	Framework::CSqliteStatement statement(m_db, "DELETE FROM shaderCap WHERE discId = ?");
	statement.BindText(1, discId);
	statement.StepNoResult();
}

ShaderCap CClient::ReadShaderCap(Framework::CSqliteStatement& statement)
{
	ShaderCap shaderCap;
	shaderCap.shaderCap = sqlite3_column_int64(statement, 0);
	return shaderCap;
}

void CClient::CheckDbVersion()
{
	bool dbExistsAndMatchesVersion =
	    [&]() {
		    try
		    {
			    auto db = Framework::CSqliteDb(Framework::PathUtils::GetNativeStringFromPath(m_dbPath).c_str(),
			                                   SQLITE_OPEN_READONLY);

			    Framework::CSqliteStatement statement(db, "PRAGMA user_version");
			    statement.StepWithResult();
			    int version = sqlite3_column_int(statement, 0);
			    return (version == DATABASE_VERSION);
		    }
		    catch(...)
		    {
			    return false;
		    }
	    }();

	if(!dbExistsAndMatchesVersion)
	{
		fs::remove(m_dbPath);
	}
}
