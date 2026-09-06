#pragma once
#include "JobQueue.h"

/*---------------------------------------
	여러개의 쓰레드가 동시에 작업할 수 있는 jobQueue
---------------------------------------*/

class ConcurrentJobQueue : public JobQueue
{
	friend class ThreadManager;

public:
	void		 SetPopCount(uint32 count) { m_popCount = count; }
	virtual void Push(RefJob job, bool pushOnly) override;
	virtual void Execute() override;

protected:
	uint32 m_popCount = 1;
};

