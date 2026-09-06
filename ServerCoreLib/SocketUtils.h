#pragma once
#include "NetAddress.h"

class SocketUtils
{
public:
	static LPFN_CONNECTEX		ConnectEx; // ptr
	static LPFN_DISCONNECTEX	DisconnectEx; // ptr
	static LPFN_ACCEPTEX		AcceptEx; // ptr

public:
	static void Init();
	static void Clear();

	static bool	  BindSocketFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateDefaultSocket();

	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	static bool SetSendBuferSize(SOCKET socket, int32 size);
	static bool SetNoDelayTCP(SOCKET socket, bool flag);
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

	// 0 입력시 남는 포트 중 아무거나 설정됨.
	static bool BindAnyAddress(SOCKET& socket, uint16 port);
	static bool BindSocket(SOCKET& socket, NetAddress netAddr);
	static bool ListenSocket(SOCKET& socket, int32 backlog = SOMAXCONN);
	static bool CloseSocket(SOCKET& socket);

};


/*------Socket Option--------*/
template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optValue)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optValue), sizeof(T));
}