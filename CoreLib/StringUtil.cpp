#include "CorePch.h"
#include "StringUtil.h"
#include <sstream>
#include <iomanip>

std::wstring ToPaddedWString(int value, int width)
{
    std::wstringstream ss;
    ss << std::setw(width) << std::setfill(L'0') << value;
    return ss.str();
}

std::wstring StringToWString(const std::string& str)
{
    size_t size = str.size();
    if (size == 0)
        return L"";

    int wsize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(size), nullptr, 0);

    std::wstring wstr;
    wstr.resize(wsize);

    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(size), &wstr[0], wsize);

    return wstr;
}

exwstring EXStringToEXWString(const exstring& str)
{
    size_t size = str.size();
    if (size == 0)
        return L"";

    int wsize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(size), nullptr, 0);

    exwstring wstr;
    wstr.resize(wsize);

    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(size), &wstr[0], wsize);

    return wstr;
}

exstring EXWStringToEXString(const exwstring& wstr)
{
    if (wstr.empty()) return "";

    int iStrLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (iStrLen <= 0) return "";

    exstring str(iStrLen, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], iStrLen, nullptr, nullptr);

    
    str.resize(iStrLen - 1);
    return str;
}
