#include "ServerCorePch.h"
#include "SocketUtils.h"

LPFN_CONNECTEX		SocketUtils::ConnectEx = nullptr; // ptr
LPFN_DISCONNECTEX	SocketUtils::DisconnectEx = nullptr; // ptr
LPFN_ACCEPTEX		SocketUtils::AcceptEx = nullptr; // ptr

void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_TRIG_CRASH(0 == ::WSAStartup(MAKEWORD(2, 2), OUT & wsaData));

	SOCKET dummy = CreateDefaultSocket();
	ASSERT_TRIG_CRASH(BindSocketFunction(dummy, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_TRIG_CRASH(BindSocketFunction(dummy, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));
	ASSERT_TRIG_CRASH(BindSocketFunction(dummy, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));

	CloseSocket(dummy);
}

void SocketUtils::Clear()
{

	::WSACleanup();
}

bool SocketUtils::BindSocketFunction(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid),
		fn, sizeof(*fn), OUT & bytes, nullptr, nullptr);
}

SOCKET SocketUtils::CreateDefaultSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)
{
	LINGER opt;
	opt.l_onoff = onoff;
	opt.l_linger = linger;

	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, opt);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBuferSize(SOCKET socket, int32 size)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetNoDelayTCP(SOCKET socket, bool flag)
{
	return SetSockOpt(socket, IPPROTO_TCP, TCP_NODELAY, flag);
}

bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

bool SocketUtils::BindAnyAddress(SOCKET& socket, uint16 port)
{
	SOCKADDR_IN Address;
	Address.sin_family = AF_INET; // IPv4
	Address.sin_addr.s_addr = ::htonl(INADDR_ANY);
	Address.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&Address), sizeof(Address));
}

bool SocketUtils::BindSocket(SOCKET& socket, NetAddress netAddr)
{
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr()), sizeof(SOCKADDR_IN));
}

bool SocketUtils::ListenSocket(SOCKET& socket, int32 backlog)
{
	return SOCKET_ERROR != ::listen(socket, backlog);
}

bool SocketUtils::CloseSocket(SOCKET& socket)
{
	if (socket == INVALID_SOCKET)
		return false;

	if (SOCKET_ERROR == ::closesocket(socket))
		return false;

	socket = INVALID_SOCKET;
	return true;
}