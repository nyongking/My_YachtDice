#include "ServerCorePch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"

Service::Service(ServiceType type, NetAddress address, RefIocpCore refCore, SessionFactory factory, int32 maxSessionCount)
	: m_serviceType(type)
	, m_netAddress(address)
	, m_refCore(refCore)
	, m_sessionFactory(factory)
	, m_maxSessionCount(maxSessionCount)
{
}

Service::~Service()
{
}

void Service::CloseService()
{
}

void Service::BroadCast(RefSendBuffer refSendBuffer)
{
	WRITE_LOCK;
	for (auto& session : m_sessions)
		session->Send(refSendBuffer);
}

RefSession Service::CreateSession() // Service마다 session을 만드는 방법, 종류가 다를것이다.
{
	RefSession refSession = m_sessionFactory();
	refSession->SetService(shared_from_this());


	if (false == m_refCore->Register(refSession))
		return nullptr;

	return refSession;
}

void Service::AddSession(RefSession refSession)
{
	WRITE_LOCK;

	++m_sessionCount;
	m_sessions.insert(refSession);

}

void Service::ReleaseSession(RefSession refSession)
{
	WRITE_LOCK;

	ASSERT_TRIG_CRASH(0 != m_sessions.erase(refSession));
	--m_sessionCount;
}

ClientService::ClientService(NetAddress targetAddress, RefIocpCore refCore, SessionFactory factory, int32 maxSessionCount)
	: Service(ServiceType::Client, targetAddress, refCore, factory, maxSessionCount)
{
}

ClientService::~ClientService()
{
}

bool ClientService::Start()
{
	if (false == CanStart())
		return false;

	const int32 sessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < sessionCount; ++i)
	{
		RefSession refSession = CreateSession();
		if (false == refSession->Connect())
			return false;
	}

	return true;
}

void ClientService::CloseService()
{
}

ServerService::ServerService(NetAddress address, RefIocpCore refCore, SessionFactory factory, int32 maxSessionCount)
	: Service(ServiceType::Server, address, refCore, factory, maxSessionCount)
{
}

ServerService::~ServerService()
{
}

bool ServerService::Start()
{
	if (false == CanStart())
		return false;

	m_listener = MakeShared<Listener>();
	if (m_listener == nullptr)
		return false;

	RefServerService refService = std::static_pointer_cast<ServerService>(shared_from_this());
	if (false == m_listener->StartAccept(refService))
		return false;

	return true;
}

void ServerService::CloseService()
{

	Service::CloseService();
}
