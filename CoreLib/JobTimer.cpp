#include "CorePch.h"
#include "JobTimer.h"
#include "JobQueue.h"

void JobTimer::Reserve(uint64 tickAfter, std::weak_ptr<JobQueue> owner, RefJob refJob)
{
	// 실행 시점을 계산
	const uint64 executeTick = GetTickCount64() + tickAfter;
	// JobData 동적생성 (어떤 JobQueue, 어떤 Job인지)
	JobData* pJobData = ObjectPool<JobData>::Pop(owner, refJob);

	WRITE_LOCK;
	// TimerItem에 추가 (실행 시점, JobQueue, Job)
	m_Items.push(TimerItem{ executeTick, pJobData });
}

// 현재 시간에 맞게 실행
void JobTimer::Distribute(uint64 now)
{
	// 이미 다른 쓰레드가 진행중이면 종료
	if (true == m_distrubuting.exchange(true))
		return;

	exvector<TimerItem> items;

	{
		WRITE_LOCK;
		while (false == m_Items.empty())
		{
			const TimerItem& timerItem = m_Items.top();
			// 실행할 때가 되지 않은 JobItem
			if (now < timerItem.m_ExecuteTick)
				break;

			// items에 실행할 (JobQueue, Job)을 추가
			items.push_back(timerItem);
			m_Items.pop();
		}
	}

	// 이제 Thread-free
	for (TimerItem& item : items)
	{
		if (RefJobQueue owner = item.m_pJobData->m_owner.lock()) // nullptr 확인
		{
			// 해당 job을 jobQueue에 추가
			owner->Push(item.m_pJobData->m_refJob, true);
		}

		// 동적할당한 JobData 반납
		ObjectPool<JobData>::Push(item.m_pJobData);
	}


	m_distrubuting.store(false);
}

void JobTimer::Clear()
{
	WRITE_LOCK;

	while (false == m_Items.empty())
	{
		const TimerItem& timerItem = m_Items.top();

		ObjectPool<JobData>::Push(timerItem.m_pJobData);
		m_Items.pop();
	}
}

bool JobTimer::HasWork()
{
	if (true == m_distrubuting.load())
		return false;

	READ_LOCK;

	return !(true == m_distrubuting.load() || true == m_Items.empty());
}