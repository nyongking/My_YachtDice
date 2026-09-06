#pragma once

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return m_iocpCore; }

	bool Register(RefIocpObject refIocpObject); // 관찰대상 등록
	bool Dispatch(uint32 timeoutInMs = INFINITE); // 워커쓰레드가 할 일을 찾는다.

private:
	HANDLE m_iocpCore;
};

