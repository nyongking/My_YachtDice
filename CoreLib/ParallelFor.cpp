#include "CorePch.h"
#include "ParallelFor.h"
#include "CountdownEvent.h"
#include "Job.h"
#include "JobQueue.h"
#include "CoreGlobal.h"

#include <thread>

void ParallelFor(int32 start, int32 end, std::function<void(int32)> body)
{
	const int32 totalCount = end - start;
	if (totalCount <= 0)
		return;

	// 하드웨어 코어 수 기반 청크 크기 결정
	const int32 workerCount = static_cast<int32>(std::thread::hardware_concurrency());
	const int32 chunkSize   = (workerCount > 1)
		? (totalCount + workerCount - 1) / workerCount
		: totalCount;

	ParallelFor(start, end, chunkSize, std::move(body));
}

void ParallelFor(int32 start, int32 end, int32 chunkSize, std::function<void(int32)> body)
{
	const int32 totalCount = end - start;
	if (totalCount <= 0)
		return;

	if (chunkSize <= 0)
		chunkSize = 1;

	// 범위가 작으면 인라인 실행 (스레드 오버헤드 없음)
	if (totalCount <= chunkSize)
	{
		for (int32 i = start; i < end; ++i)
			body(i);
		return;
	}

	// 청크 수 계산
	const int32 chunkCount = (totalCount + chunkSize - 1) / chunkSize;

	// 스택에 CountdownEvent 생성 — 모든 청크가 Signal()하면 Wait() 반환
	CountdownEvent countdown(chunkCount);
	CountdownEvent* pCountdown = &countdown;

	// 청크별로 JobQueue를 하나씩 만들어 글로벌 워커 풀에 제출
	for (int32 chunk = 0; chunk < chunkCount; ++chunk)
	{
		const int32 chunkStart = start + chunk * chunkSize;
		const int32 chunkEnd = (chunkStart + chunkSize < end)
			? (chunkStart + chunkSize)
			: end;

		auto chunkQueue = std::make_shared<JobQueue>();

		chunkQueue->DoAsync(
			/*pushOnly=*/true,
			[pCountdown, chunkStart, chunkEnd, body]()
			{
				for (int32 i = chunkStart; i < chunkEnd; ++i)
					body(i);

				pCountdown->Signal();
			}
		);
	}

	// 모든 청크 완료까지 현재 스레드 블로킹 대기
	countdown.Wait();
}
