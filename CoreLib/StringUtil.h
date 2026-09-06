#pragma once
#include <string>


std::wstring ToPaddedWString(int value, int width);

std::wstring StringToWString(const std::string& str);

exwstring EXStringToEXWString(const exstring& str);

exstring EXWStringToEXString(const exwstring& wstr);
