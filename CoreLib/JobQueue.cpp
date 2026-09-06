#include "CorePch.h"
#include "JobQueue.h"

void JobQueue::Push(RefJob job, bool pushOnly)
{
	// 아래의 순서를 꼭 지켜야 한다!!
	const int32 prevCount = m_jobCount.fetch_add(1); // 카운트 증가 후 @@
	m_jobQueue.Push(job); // jobQueue에 job 추가, 이 단계에서 LOCK

	// Queue에 추가한 이후, 이전의 카운트가 0이면 Execute 호출
	// 즉, 이전에 Execute를 호출한 쓰레드가 없었다면 해당 쓰레드가 Execute를 호출
	if (0 == prevCount)
	{
		if (nullptr == LCurrentJobQueue && false == pushOnly)
		{
			Execute();
		}
		else
		{
			PushGlobalQueue(shared_from_this());
		}
	}
}

void JobQueue::Execute()
{
	LCurrentJobQueue = this;

	while (true)
	{
		exvector<RefJob> jobs;

		m_jobQueue.PopAll(OUT jobs); // Job들을 모두 꺼내옴

		// 꺼내온 Job들을 모두 실행
		const uint32 jobCount = static_cast<uint32>(jobs.size());
		for (uint32 i = 0; i < jobCount; ++i)
			jobs[i]->Execute();

		// 원래 JobCount에서 꺼내온 JobCount만큼 빼기
		// 빼고 난 후의 값이 0이면 종료 (모든 Job 처리 완료)
		if (jobCount == m_jobCount.fetch_sub(jobCount))
		{
			LCurrentJobQueue = nullptr;
			return;
		}

		// 하나의 쓰레드에 몰림현상 해결
		const uint64 now = ::GetTickCount64();
		if (now >= LWorkerEndTick)
		{
			LCurrentJobQueue = nullptr;

			PushGlobalQueue(shared_from_this());

			break;
		}

	}
}

void JobQueue::ReserveGlobalTimer(uint64 tickAfter, RefJob job)
{
	// TODO: Global JobTimer로 옮기기

}
