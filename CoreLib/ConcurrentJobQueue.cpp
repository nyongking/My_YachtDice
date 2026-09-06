#include "CorePch.h"
#include "ConcurrentJobQueue.h"

void ConcurrentJobQueue::Push(RefJob job, bool pushOnly)
{
	m_jobCount.fetch_add(1);
	m_jobQueue.Push(job);

	if (nullptr == LCurrentJobQueue && false == pushOnly)
	{
		Execute();
	}
	else
	{
		PushGlobalQueue(shared_from_this());
	}
}

void ConcurrentJobQueue::Execute()
{
	LCurrentJobQueue = this;

	exvector<RefJob> jobs;
	m_jobQueue.PopSize(m_popCount, OUT jobs);

	const uint32 jobCount = static_cast<uint32>(jobs.size());
	for (uint32 i = 0; i < jobCount; ++i)
		jobs[i]->Execute();

	m_jobCount.fetch_sub(jobCount);
	LCurrentJobQueue = nullptr;
}
