#include "CorePch.h"
#include "CoreGlobal.h"

#include "Memory.h"
#include "ThreadManager.h"
#include "DeadLockProfiler.h"
#include "MainThreadQueue.h"

Memory* GMemory = nullptr;
DeadLockProfiler* GDeadLockProfiler = nullptr;
ThreadManager* GThreadManager = nullptr;
MainThreadQueue* GMainQueue = nullptr;

class CoreGlobal
{
public:
	CoreGlobal();
	~CoreGlobal();

	static CoreGlobal* sInstance;
};

CoreGlobal* CoreGlobal::sInstance = nullptr;

CoreGlobal::CoreGlobal()
{
	GMemory = new Memory();
	GThreadManager = new ThreadManager();
	GDeadLockProfiler = new DeadLockProfiler();
	GMainQueue = new MainThreadQueue();


}

CoreGlobal::~CoreGlobal()
{
	delete GMainQueue;
	delete GThreadManager;
	delete GDeadLockProfiler;

	delete GMemory;
}


void InitCore()
{
	if (nullptr == CoreGlobal::sInstance)
		CoreGlobal::sInstance = new CoreGlobal();
}

void ReleaseCore()
{
	if (nullptr != CoreGlobal::sInstance)
		delete CoreGlobal::sInstance;
}

void PushGlobalQueue(RefJobQueue jobQueue)
{
	GThreadManager->PushJob(jobQueue);
}

void PushGlobalTimer(uint64 tickAfter, std::weak_ptr<class JobQueue> refOwner, RefJob refJob)
{
	GThreadManager->PushTimer(tickAfter, refOwner, refJob);
}

void DoGlobalWork(uint64 tickCount, uint64 tickWait)
{
	GThreadManager->DoGlobalWork(tickCount, tickWait);
}

