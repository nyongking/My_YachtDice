#include "CorePch.h"
#include "ThreadManager.h"
#include "JobQueueContainer.h"
#include "JobQueue.h"

ThreadManager::ThreadManager()
{
	// 메인 쓰레드의 초기화
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();


}

void ThreadManager::Launch(std::function<void(void)> callback)
{
	LockGuard guard(m_managerLock);

	m_threads.push_back(std::thread([=]()
		{
			InitTLS();
			callback();
			DestroyTLS();
		}));
}

void ThreadManager::Join()
{
	for (std::thread& th : m_threads)
	{
		if (th.joinable())
			th.join();
	}

	m_threads.clear();
}

void ThreadManager::InitTLS()	// TLS 초기화
{
	static Atomic<uint32> SThreadID = 1;

	LThreadID = SThreadID.fetch_add(1);
}

void ThreadManager::DestroyTLS() // TLS 삭제
{
}

void ThreadManager::DoGlobalQueueWork()
{
	while (true)
	{
		uint64 now = ::GetTickCount64();
		if (now > LWorkerEndTick)
			break;

		RefJobQueue refJobQueue = m_globalQueue.Pop();
		if (nullptr == refJobQueue)
		{
			// 할 작업이 존재하지 않아.
			break;
		}

		refJobQueue->Execute();
	}
}

void ThreadManager::DistributeReserveJobs()
{
	const uint64 now = ::GetTickCount64();

	m_globalTimer.Distribute(now);

	// temp
	//chrono::duration<int, milli> duration(10);
	//this_thread::sleep_for(duration);
}

void ThreadManager::DoGlobalWork(uint64 tickCount, uint64 tickWait)
{
	LWorkerEndTick = ::GetTickCount64() + tickCount;

	// 할 일이 존재할 경우
	if (m_globalQueue.HasWork() || m_globalTimer.HasWork())
	{
		DoGlobalQueueWork();
		DistributeReserveJobs();
	}
	// GlobalQueue의 일이 없고, JobTimer의 할 일이 없을 경우
	else
	{
		//unique_lock<Mutex> lock(m_threadLock);

		std::chrono::duration<uint64, std::milli> duration(tickWait);

		//	m_threadCondition.wait_for(lock, duration,
		//		[&]() { return m_globalQueue.HasWork() || m_globalTimer.HasWork(); });

		// 단순 sleep으로 대체
		std::this_thread::sleep_for(duration);
	}

}

void ThreadManager::PushJob(RefJobQueue jobQueue)
{
	m_globalQueue.Push(jobQueue);
}

void ThreadManager::PushTimer(uint64 tickAfter, std::weak_ptr<JobQueue> refOwner, RefJob refJob)
{
	m_globalTimer.Reserve(tickAfter, refOwner, refJob);
}
