#include <memory>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <nlohmann/json.hpp>
#include "TheGamesDbClient.h"
#include "string_format.h"
#include "http/HttpClientFactory.h"

using namespace TheGamesDb;

static const char *g_getGameUrl = "https://api.thegamesdb.net/Games/GamesByGameID?apikey=API_KEY&fields=overview,serials&include=boxart&id=%d";
static const char *g_getGamesBySerialUrl = "https://api.thegamesdb.net/Games/ByGameSerialID?apikey=API_KEY&filter%5Bplatform%5D=11&fields=overview,serials&include=boxart";
static const char* g_getGamesListUrl = "https://api.thegamesdb.net/v1.1/Games/ByGameName?apikey=API_KEY&fields=overview,serials&filter%5Bplatform%5D=%s&include=boxart&name=%s";

GamesList CClient::GetGames(std::vector<std::string> serials)
{
	std::ostringstream stream;
	std::copy(serials.begin(), serials.end(), std::ostream_iterator<std::string>(stream, ","));
	std::string str_games_id = stream.str();

	std::vector<Game> gamesList;

	auto url = std::string(g_getGamesBySerialUrl);
	url += "&id=";
	url += str_games_id;
	while(!url.empty())
	{
		auto requestResult =
		    [&]() {
			    auto client = Framework::Http::CreateHttpClient();
			    client->SetUrl(url);
			    return client->SendRequest();
		    }();
		url.clear();
		if(requestResult.statusCode == Framework::Http::HTTP_STATUS_CODE::OK)
		{
			auto json_ret = requestResult.data.ReadString();
			auto parsed_json = nlohmann::json::parse(json_ret);

			PopulateGameList(json_ret, gamesList, url);
		}
	}
	return gamesList;
}

Game CClient::GetGame(uint32 id)
{
	auto url = string_format(g_getGameUrl, id);
	auto requestResult =
	    [&]() {
		    auto client = Framework::Http::CreateHttpClient();
		    client->SetUrl(url);
		    return client->SendRequest();
	    }();

	if(requestResult.statusCode != Framework::Http::HTTP_STATUS_CODE::OK)
	{
		throw std::runtime_error("Failed to get game.");
	}

	auto json_ret = requestResult.data.ReadString();
	std::vector<Game> gamesList;
	int count = PopulateGameList(json_ret, gamesList);

	if(count < 1)
	{
		throw std::runtime_error("Failed to get game.");
	}
	Game game;

	return gamesList.at(0);;
}

GamesList CClient::GetGamesList(const std::string& platformID, const std::string& name)
{
	auto encodedName = Framework::Http::CHttpClient::UrlEncode(name);

	auto url = string_format(g_getGamesListUrl, platformID.c_str(), encodedName.c_str());
	auto requestResult =
	    [&]() {
		    auto client = Framework::Http::CreateHttpClient();
		    client->SetUrl(url);
		    return client->SendRequest();
	    }();

	if(requestResult.statusCode != Framework::Http::HTTP_STATUS_CODE::OK)
	{
		throw std::runtime_error("Failed to get games list.");
	}

	auto json_ret = requestResult.data.ReadString();
	std::vector<Game> gamesList;
	int count = PopulateGameList(json_ret, gamesList);

	if (count < 1)
	{
		throw std::runtime_error("Failed to get game.");
	}

	return gamesList;
}

int CClient::PopulateGameList(std::string &json_ret, std::vector<TheGamesDb::Game> &list)
{
	std::string tmp;
	return PopulateGameList(json_ret, list, tmp);
}

int CClient::PopulateGameList(std::string &json_ret, std::vector<TheGamesDb::Game> &list, std::string &next_page_url)
{
	nlohmann::json parsed_json = nlohmann::json::parse(json_ret);

	if(parsed_json["data"]["count"].get<int>() == 0)
		return 0;

	if(!parsed_json["pages"]["next"].empty())
		next_page_url = parsed_json["pages"]["next"].get<std::string>();

	auto games = parsed_json["data"]["games"].get<std::vector<nlohmann::json>>();

	std::string image_base = "";
	auto includes = parsed_json["include"];
	if(!includes.empty())
	{
		image_base = includes["boxart"]["base_url"]["medium"].get<std::string>();
	}
	int count = 0;
	for(auto game : games)
	{
		++count;
		int game_id = game["id"].get<int>();
		TheGamesDb::Game meta;
		meta.id = game_id;
		meta.overview = game["overview"].get<std::string>();
		meta.discIds = game["serials"].get<std::vector<std::string>>();
		meta.title = game["game_title"].get<std::string>();
		meta.baseImgUrl = image_base;
		if(!includes.empty())
		{
			auto boxarts = includes["boxart"]["data"];
			if(!boxarts.empty())
			{
				auto str_id = std::to_string(game_id);
				auto games_cover_meta = boxarts[str_id.c_str()];
				if(!games_cover_meta.empty())
				{
					for(auto &game_cover : games_cover_meta)
					{

						meta.boxArtUrl = game_cover["filename"].get<std::string>().c_str();
						if(game_cover["side"].get<std::string>() == "front")
						{
							break;
						}
					}
				}
			}
		}
		list.push_back(meta);
	}
	return count;
}
