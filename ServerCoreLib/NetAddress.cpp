#include "ServerCorePch.h"
#include "NetAddress.h"

NetAddress::NetAddress(SOCKADDR_IN sockAddrIn)
	: m_sockAddrIn(sockAddrIn)
{
}

NetAddress::NetAddress(std::wstring ip, uint16 port)
{
	memset(&m_sockAddrIn, 0, sizeof(m_sockAddrIn));
	m_sockAddrIn.sin_family = AF_INET;
	m_sockAddrIn.sin_addr = IPToAddress(ip.c_str());
	m_sockAddrIn.sin_port = ::htons(port);
}

std::wstring NetAddress::GetIpAddress()
{
	TCHAR buffer[64];

	::InetNtopW(AF_INET, &m_sockAddrIn.sin_addr, buffer, len32(buffer));
	return std::wstring(buffer);
}

IN_ADDR NetAddress::IPToAddress(const TCHAR* ip)
{
	IN_ADDR address;
	::InetPtonW(AF_INET, ip, &address);
	return address;
}
