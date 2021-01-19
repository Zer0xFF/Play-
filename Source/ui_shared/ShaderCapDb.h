#pragma once

#include <string>
#include <vector>
#include "filesystem_def.h"
#include "Types.h"
#include "Singleton.h"
#include "sqlite/SqliteDb.h"
#include "sqlite/SqliteStatement.h"

namespace ShaderCapDb
{
	struct ShaderCap
	{
		int id;
		std::string discId;
		uint32_t shaderCap = 0;
	};

	class CClient : public CSingleton<CClient>
	{
	public:

		CClient();
		virtual ~CClient() = default;

		std::vector<ShaderCap> GetShaderCaps(const char*);

		void RegisterShaderCap(const char*, uint32_t);
		void UnregisterShaderCap(const char*);


	private:
		static ShaderCap ReadShaderCap(Framework::CSqliteStatement&);

		void CheckDbVersion();

		fs::path m_dbPath;
		Framework::CSqliteDb m_db;
	};
};
