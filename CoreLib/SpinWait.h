#pragma once

/*---------------------------------------
	SpinWait
	- 단계적 대기 전략 유틸리티
	- Phase 1 (0~9):  YieldProcessor()   — 동일 코어, CPU 힌트
	- Phase 2 (10~19): SwitchToThread()  — 동일 우선순위 스레드에 양보
	- Phase 3 (20+):  Sleep(1)           — 타임슬라이스 양보
---------------------------------------*/

class SpinWait
{
public:
	SpinWait() = default;
	~SpinWait() = default;

	// 복사 금지
	SpinWait(const SpinWait&) = delete;
	SpinWait& operator=(const SpinWait&) = delete;

public:
	// 한 번 대기 수행, 반복 횟수에 따라 전략 자동 전환
	void SpinOnce()
	{
		if (m_count < YIELD_THRESHOLD)
		{
			// Phase 1: CPU 힌트 (같은 코어에서 짧게 대기)
			YieldProcessor();
		}
		else if (m_count < SLEEP_THRESHOLD)
		{
			// Phase 2: 동일 우선순위 스레드에 실행 양보
			::SwitchToThread();
		}
		else
		{
			// Phase 3: OS 타임슬라이스 양보 (다른 스레드/코어 실행 허용)
			::Sleep(1);
		}

		++m_count;
	}

	// 다음 SpinOnce()가 OS yield를 수행하는지 확인 (iteration >= YIELD_THRESHOLD)
	bool NextSpinWillYield() const
	{
		return m_count >= YIELD_THRESHOLD;
	}

	// 반복 카운터 초기화
	void Reset()
	{
		m_count = 0;
	}

	// 현재 반복 횟수 반환
	int32 Count() const
	{
		return m_count;
	}

private:
	static constexpr int32 YIELD_THRESHOLD = 10;
	static constexpr int32 SLEEP_THRESHOLD = 20;

	int32 m_count = 0;
};
