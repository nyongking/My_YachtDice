#pragma once

/*---------------------------------------
	ParallelFor
	- 범위 [start, end)를 청크로 분할해 워커 스레드에서 병렬 실행
	- 내부적으로 CountdownEvent + Job 사용
	- 범위가 작으면 (≤ chunkSize) 인라인 직접 실행
---------------------------------------*/

// 자동 청크 크기 (워커 수 기반)
void ParallelFor(int32 start, int32 end, std::function<void(int32)> body);

// 수동 청크 크기 지정
void ParallelFor(int32 start, int32 end, int32 chunkSize, std::function<void(int32)> body);
