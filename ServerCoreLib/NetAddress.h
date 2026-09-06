#pragma once

class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockAddrIn);
	NetAddress(std::wstring ip, uint16 port);

public:
	SOCKADDR_IN& GetSockAddr() { return m_sockAddrIn; }
	std::wstring			GetIpAddress();
	uint16			GetPort() { return ::ntohs(m_sockAddrIn.sin_port); }

	static IN_ADDR IPToAddress(const TCHAR* ip);

private:
	SOCKADDR_IN m_sockAddrIn = {};
};

