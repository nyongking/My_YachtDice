#include "ServerCorePch.h"
#include "IocpCore.h"
#include "IocpObject.h"
#include "OverlappedEx.h"

IocpCore::IocpCore()
{
	m_iocpCore = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_TRIG_CRASH(m_iocpCore != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(m_iocpCore);
}

bool IocpCore::Register(RefIocpObject refIocpObject)
{
	return INVALID_HANDLE_VALUE != ::CreateIoCompletionPort(refIocpObject->GetHandle(), m_iocpCore,
		0, 0);
}

bool IocpCore::Dispatch(uint32 timeoutInMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	OverlappedEx* pOverlappedEx = nullptr;

	// 현재 key로 넘겨주는 IocpObject가 존재하지 않는다면?
	if (::GetQueuedCompletionStatus(m_iocpCore, OUT & numOfBytes, OUT & key,
		OUT reinterpret_cast<LPOVERLAPPED*>(&pOverlappedEx), timeoutInMs))
	{
		RefIocpObject owner = pOverlappedEx->m_owner;
		owner->Dispatch(pOverlappedEx, numOfBytes);
	}
	else
	{
		// fail

		int32 errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			RefIocpObject owner = pOverlappedEx->m_owner;
			owner->Dispatch(pOverlappedEx, numOfBytes);
			break;
		}
	}

	return false;
}
