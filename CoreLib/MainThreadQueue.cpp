#include "CorePch.h"
#include "MainThreadQueue.h"

MainThreadQueue::MainThreadQueue()
{
}

MainThreadQueue::~MainThreadQueue()
{
}

void MainThreadQueue::Execute(uint32 size)
{
	exvector<RefJob> jobs;

	m_jobQueue.PopSize(size, OUT jobs);
	const uint32 jobCount = static_cast<uint32>(jobs.size());

	for (uint32 i = 0; i < jobCount; ++i)
		jobs[i]->Execute();
}

void MainThreadQueue::ExecuteAll()
{
	exvector<RefJob> jobs;

	m_jobQueue.PopAll(OUT jobs);
	const uint32 jobCount = static_cast<uint32>(jobs.size());

	for (uint32 i = 0; i < jobCount; ++i)
		jobs[i]->Execute();
}

void MainThreadQueue::Push(RefJob job)
{
	m_jobQueue.Push(job);
}
