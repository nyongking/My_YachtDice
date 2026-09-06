#include "CorePch.h"
#include "JsonUtil.h"

bool LoadJson(const WCHAR* path, OUT MyJson& json)
{
	std::ifstream inputFile(path);

	if (!inputFile.is_open())
	{
		return false;
	}

	inputFile >> json;
	inputFile.close();
	return true;
}

bool LoadJson(const CHAR* path, OUT MyJson& json)
{
	std::ifstream inputFile(path);

	if (!inputFile.is_open())
	{
		return false;
	}

	inputFile >> json;
	inputFile.close();
	return true;
}

uint32 JsonCount(const MyJson& json, const CHAR* jsonKey)
{
	if (json.contains(jsonKey))
		return static_cast<uint32>(json[jsonKey].size());
	else
		return 0;
}



