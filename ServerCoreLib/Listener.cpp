#include "ServerCorePch.h"
#include "Listener.h"
#include "OverlappedEx.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "Session.h"
#include "Service.h"


Listener::~Listener()
{
	SocketUtils::CloseSocket(m_socket);

	for (OverlappedAccept* pAccept : m_OverlappedAccepts)
	{
		exdelete(pAccept);
	}
}

bool Listener::StartAccept(RefServerService refServerService)
{
	if (nullptr == refServerService)
		return false;

	m_ServerService = refServerService;

	m_socket = SocketUtils::CreateDefaultSocket();

	if (INVALID_SOCKET == m_socket)
		return false;

	if (false == SocketUtils::SetReuseAddress(m_socket, true))
		return false;

	if (false == SocketUtils::SetLinger(m_socket, 0, 0))
		return false;

	if (false == SocketUtils::BindSocket(m_socket, m_ServerService->GetNetAddress()))
		return false;

	if (false == SocketUtils::ListenSocket(m_socket))
		return false;

	if (false == m_ServerService->GetIocpCore()->Register(shared_from_this())) // 관찰등록
		return false;

	const int32 acceptCount = m_ServerService->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; ++i)
	{
		OverlappedAccept* pAccept = exnew<OverlappedAccept>();
		pAccept->m_owner = shared_from_this();
		m_OverlappedAccepts.push_back(pAccept);
		RegisterAccept(pAccept);
	}


	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::CloseSocket(m_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(m_socket);
}

// 일감에 따라 할일을 한다.
void Listener::Dispatch(OverlappedEx* pOverlappedEx, int32 numofBytes)
{
	ASSERT_TRIG_CRASH(pOverlappedEx->m_EventType == NetworkEventType::Accept);
	OverlappedAccept* pAccept = static_cast<OverlappedAccept*>(pOverlappedEx);
	ProcessAccept(pAccept);

}

// Accept !
void Listener::RegisterAccept(OverlappedAccept* pAccept)
{
	RefSession refSession = m_ServerService->CreateSession(); //Session을 여기서 추가? or Process에서 추가?

	pAccept->Init();
	pAccept->m_refSession = refSession;

	DWORD BytesRecv;

	if (false == SocketUtils::AcceptEx(m_socket, /*reinterpret_cast<SOCKET>(pSession->GetHandle())*/
		refSession->GetSocket(),
		refSession->m_recvBuffer.WritePos(),
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		OUT & BytesRecv,
		static_cast<OVERLAPPED*>(pAccept)
	))
	{
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			RegisterAccept(pAccept);
		}
	}
}

void Listener::ProcessAccept(OverlappedAccept* pAccept)
{
	RefSession refSession = pAccept->m_refSession;

	if (false == SocketUtils::SetUpdateAcceptSocket(refSession->GetSocket(), m_socket))
	{
		RegisterAccept(pAccept);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sockAddressSize = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(refSession->GetSocket(),
		OUT reinterpret_cast<SOCKADDR*>(&sockAddress),
		&sockAddressSize
	)) // 어떤 클라이언트?
	{
		RegisterAccept(pAccept);
		return;
	}

	refSession->SetNetAddress(NetAddress(sockAddress));

	refSession->ProcessConnect(); // Session의 연결처리


	RegisterAccept(pAccept);
}
