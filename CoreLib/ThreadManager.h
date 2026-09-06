#pragma once
#include <thread>
#include <functional>
#include "JobQueueContainer.h"
#include "JobTimer.h"

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(std::function<void(void)> callback);
	void Join();

	static void InitTLS();
	static void DestroyTLS();

	void DoGlobalQueueWork();
	void DistributeReserveJobs();
	void DoGlobalWork(uint64 tickCount, uint64 tickWait);

	void PushJob(RefJobQueue jobQueue);
	void PushTimer(uint64 tickAfter, std::weak_ptr<JobQueue> refOwner, RefJob refJob);

private:
	Mutex				m_managerLock;
	//Mutex				m_threadLock;
	//condition_variable	m_threadCondition;

	std::vector<std::thread>	m_threads;
	JobQueueContainer		m_globalQueue;
	JobTimer		m_globalTimer;
};

