#pragma once
#include "json.hpp"
#include <fstream>
#include "StringUtil.h"

using MyJson = nlohmann::json;

bool				LoadJson(const WCHAR* path, OUT MyJson& json);
bool				LoadJson(const CHAR* path, OUT MyJson& json);


template<typename T>
bool				JsonToType(const MyJson& json, const CHAR* jsonKey, OUT T& data);

template<>
bool				JsonToType<std::wstring>(const MyJson& json, const CHAR* jsonKey, OUT std::wstring& data);

template<>
bool				JsonToType<exwstring>(const MyJson& json, const CHAR* jsonKey, OUT exwstring& data);

template<typename T>
void				JsonToTypes(const MyJson& json, const CHAR* jsonKey, uint32 count, OUT T* data);

template<typename T>
void				JsonToTypes(const MyJson& json, uint32 count, OUT T* data);


uint32				JsonCount(const MyJson& json, const CHAR* jsonKey);


template<typename T>
inline bool JsonToType(const MyJson& json, const CHAR* jsonKey, OUT T& data)
{
	if (json.contains(jsonKey))
	{
		data = json[jsonKey];
		return true;
	}
	return false;
}

template<>
inline bool JsonToType(const MyJson& json, const CHAR* jsonKey, OUT std::wstring& data)
{
	if (json.contains(jsonKey))
	{
		data = StringToWString(json[jsonKey]);
		return true;
	}
	return false;
}

template<>
inline bool JsonToType(const MyJson& json, const CHAR* jsonKey, OUT exwstring& data)
{
	if (json.contains(jsonKey))
	{
		data = EXStringToEXWString(json[jsonKey]);
		return true;
	}
	return false;
}

template<typename T>
inline void JsonToTypes(const MyJson& json, const CHAR* jsonKey, uint32 count, OUT T* data)
{
	if (nullptr == data)
		return;

	if (json.contains(jsonKey))
	{
		for (uint32 i = 0; i < count; ++i)
		{
			*(data + i) = json[jsonKey][i];
		}
	}
}


template<typename T>
inline void JsonToTypes(const MyJson& json, uint32 count, OUT T* data)
{
	if (nullptr == data)
		return;

	for (uint32 i = 0; i < count; ++i)
	{
		*(data + i) = json[i];
	}
}
