#pragma once
#include "LockQueue.h"

class JobQueueContainer
{
public:
	JobQueueContainer();
	~JobQueueContainer();

public:
	void		Push(RefJobQueue refJobQueue);
	RefJobQueue Pop();
	bool		HasWork();

private:
	LockQueue<RefJobQueue> m_jobQueues;
};

