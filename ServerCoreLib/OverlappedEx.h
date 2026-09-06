#pragma once
enum class NetworkEventType : uint8
{
	Connect,
	Accept,
	Recv,
	Send,
	Disconnect,
};

class Session;

/* virtual ±ÝÁö */
class OverlappedEx : public OVERLAPPED
{
public:
	OverlappedEx(NetworkEventType type);

	void Init();
public:
	NetworkEventType	m_EventType;
	RefIocpObject		m_owner;
};

class OverlappedConnect : public OverlappedEx
{
public:
	OverlappedConnect() : OverlappedEx(NetworkEventType::Connect) {}



};

class OverlappedAccept : public OverlappedEx
{


public:
	OverlappedAccept() : OverlappedEx(NetworkEventType::Accept) {}

public:
	RefSession m_refSession = nullptr;

};


class OverlappedSend : public OverlappedEx
{
public:
	OverlappedSend() : OverlappedEx(NetworkEventType::Send) {}


	std::vector<RefSendBuffer> m_sendBuffers;
};

class OverlappedRecv : public OverlappedEx
{
public:
	OverlappedRecv() : OverlappedEx(NetworkEventType::Recv) {}



};


class OverlappedDisconnect : public OverlappedEx
{
public:
	OverlappedDisconnect() : OverlappedEx(NetworkEventType::Disconnect) {}



};
