#pragma once
#include "IocpObject.h"
#include "NetAddress.h"
#include "OverlappedEx.h"
#include "RecvBuffer.h"


class Service;

const int32 MaxSendSize = 0x10000;

class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

public:
	Session();
	virtual ~Session();

public:
	void				Send(RefSendBuffer refSendbuffer);
	bool				Connect();
	void				Disconnect(const WCHAR* cause);

	std::shared_ptr<Service>	GetService() { return m_refService.lock(); }
	void				SetService(std::shared_ptr<Service> refService) { m_refService = refService; }

public:
	void			SetNetAddress(NetAddress netAddress) { m_netAddress = netAddress; }
	NetAddress		GetNetAddress() { return m_netAddress; }
	SOCKET			GetSocket() { return m_socket; }
	bool			IsConnected() { return m_Connected; }
	RefSession		GetSession() { return std::static_pointer_cast<Session>(shared_from_this()); }

protected:
	virtual int32	OnRecv(BYTE* pBuffer, int32 len) { return len; }
	virtual void	OnSend(int32 len) {}
	virtual	void	OnConnected() {}
	virtual void	OnDisconncted() {}

private:
	virtual HANDLE	GetHandle() override;
	virtual void	Dispatch(OverlappedEx* pOverlappedEx, int32 numofBytes) override;

private:
	bool			RegisterConnect();
	bool			RegisterDisconnect();
	void			RegisterRecv();
	void			RegisterSend();

	void			ProcessConnect();
	void			ProcessDisconnect();
	void			ProcessRecv(int32 numofBytes);
	void			ProcessSend(int32 numofBytes);

	void			HandleError(int32 errorCode);


private:
	std::weak_ptr<Service>	m_refService;
	SOCKET				m_socket = INVALID_SOCKET;
	NetAddress			m_netAddress = {};
	Atomic<bool>		m_Connected = false;

	USE_LOCK;

	RecvBuffer			m_recvBuffer;

	OverlappedDisconnect m_overlappedDisconnect;
	OverlappedConnect	m_overlappedConnect;
	OverlappedRecv		m_overlappedRecv;
	OverlappedSend		m_overlappedSend;

	exqueue<RefSendBuffer>	m_sendBufferQueue;
	Atomic<bool>			m_sendRegistered = false;
};


struct PacketHeader
{
	uint16 size;
	uint16 id;
};

class PacketSession : public Session
{
public:
	PacketSession();
	virtual ~PacketSession();

	RefPacketSession GetRefPacketSession() { return std::static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32	OnRecv(BYTE* pBuffer, int32 len) sealed;
	virtual void	OnRecvPacket(BYTE* pBuffer, int32 len) abstract;


};