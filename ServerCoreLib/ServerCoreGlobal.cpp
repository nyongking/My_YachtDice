#include "ServerCorePch.h"
#include "ServerCoreGlobal.h"
#include "SendBuffer.h"

SendBufferManager* GSendBufferManager = nullptr;

class ServerCoreGlobal
{
public:
	ServerCoreGlobal();
	~ServerCoreGlobal();

public:
	static ServerCoreGlobal* sInstance;
};

ServerCoreGlobal* ServerCoreGlobal::sInstance = nullptr;

ServerCoreGlobal::ServerCoreGlobal()
{
	GSendBufferManager = new SendBufferManager;
}

ServerCoreGlobal::~ServerCoreGlobal()
{
	delete GSendBufferManager;
}

void InitServerCore()
{
	if (nullptr == ServerCoreGlobal::sInstance)
	{
		ServerCoreGlobal::sInstance = new ServerCoreGlobal();
	}
}

void ReleaseServerCore()
{
	if (nullptr != ServerCoreGlobal::sInstance)
	{
		delete ServerCoreGlobal::sInstance;
		ServerCoreGlobal::sInstance = nullptr;
	}
}
