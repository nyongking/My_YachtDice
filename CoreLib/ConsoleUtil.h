#pragma once

template<typename... Args>
void ConsoleLogW(Args&&... args);

template<typename... Args>
void ConsoleLog(Args&&... args);

template<typename ...Args>
inline void ConsoleLogW(Args&& ...args)
{
	((std::wcout << args << L" "), ...) << endl;
}

template<typename ...Args>
inline void ConsoleLog(Args && ...args)
{
	((std::cout << args << " "), ...) << endl;
}