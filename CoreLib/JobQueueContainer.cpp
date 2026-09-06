#include "CorePch.h"
#include "JobQueueContainer.h"

JobQueueContainer::JobQueueContainer()
{
}

JobQueueContainer::~JobQueueContainer()
{
}

void JobQueueContainer::Push(RefJobQueue refJobQueue)
{
	m_jobQueues.Push(refJobQueue);
}

RefJobQueue JobQueueContainer::Pop()
{
	return m_jobQueues.Pop();
}

bool JobQueueContainer::HasWork()
{
	return false == m_jobQueues.Empty();
}
