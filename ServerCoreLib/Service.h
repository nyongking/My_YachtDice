#pragma once
#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"
#include <functional>

enum class ServiceType : uint8
{
	Server,
	Client,
};

using SessionFactory = std::function<RefSession(void)>; // Session을 만드는 함수

class Service : public std::enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetAddress address, RefIocpCore refCore,
		SessionFactory factory, int32 maxSessionCount = 1);

	virtual ~Service();

public:
	bool			CanStart() const { return m_sessionFactory != nullptr; }
	void			SetSessionFactory(SessionFactory func) { m_sessionFactory = func; }

	virtual bool	Start() abstract;
	virtual void	CloseService();

	void			BroadCast(RefSendBuffer refSendBuffer);

	RefSession		CreateSession();
	void			AddSession(RefSession refSession);
	void			ReleaseSession(RefSession refSession);
	int32			GetCurrentSessionCount() const { return m_sessionCount; }
	int32			GetMaxSessionCount() const { return m_maxSessionCount; }

	ServiceType		GetServiceType() const { return m_serviceType; }
	NetAddress		GetNetAddress() const { return m_netAddress; }
	RefIocpCore& GetIocpCore() { return m_refCore; }

protected:
	USE_LOCK;

	ServiceType		m_serviceType;
	NetAddress		m_netAddress;
	RefIocpCore		m_refCore;

	exset<RefSession>	m_sessions;
	int32				m_sessionCount = 0;
	int32				m_maxSessionCount = 0;
	SessionFactory		m_sessionFactory;
};

class ClientService : public Service
{
public:
	ClientService(NetAddress targetAddress, RefIocpCore refCore, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ClientService();

public:
	virtual bool	Start() override;
	virtual void	CloseService() override;
};


class ServerService : public Service
{
public:
	ServerService(NetAddress address, RefIocpCore refCore, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ServerService();

public:
	virtual bool	Start() override;
	virtual void	CloseService() override;

private:
	RefListener		m_listener;
};

