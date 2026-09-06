#pragma once

/*---------------------------------------
	CountdownEvent
	- count개의 비동기 작업이 완료될 때까지 대기하는 동기화 프리미티브
	- Signal() 호출마다 카운터 감소, 0이 되면 이벤트 신호
	- Wait()로 블로킹 대기, WaitForSingleObject 사용 (스핀락 아님)
---------------------------------------*/

class CountdownEvent
{
public:
	explicit CountdownEvent(int32 count)
		: m_count(count)
	{
		BOOL initialState = (count <= 0) ? TRUE : FALSE;
		m_event = ::CreateEvent(nullptr, TRUE, initialState, nullptr);
		ASSERT_TRIG_CRASH(m_event != nullptr);
	}

	~CountdownEvent()
	{
		if (m_event != nullptr)
		{
			::CloseHandle(m_event);
			m_event = nullptr;
		}
	}

	// 복사/이동 금지
	CountdownEvent(const CountdownEvent&) = delete;
	CountdownEvent& operator=(const CountdownEvent&) = delete;
	CountdownEvent(CountdownEvent&&) = delete;
	CountdownEvent& operator=(CountdownEvent&&) = delete;

public:
	// 카운터를 1 감소시킨다. 0에 도달하면 이벤트 신호
	void Signal()
	{
		const int32 prev = m_count.fetch_sub(1);
		if (prev == 1)
		{
			::SetEvent(m_event);
		}
	}

	// 카운터가 0이 될 때까지 무한 대기
	void Wait()
	{
		if (m_count.load() <= 0)
			return;

		::WaitForSingleObject(m_event, INFINITE);
	}

	// 타임아웃(밀리초) 지정 대기. true = 완료, false = 타임아웃
	bool Wait(uint32 timeoutMs)
	{
		if (m_count.load() <= 0)
			return true;

		DWORD result = ::WaitForSingleObject(m_event, static_cast<DWORD>(timeoutMs));
		return (result == WAIT_OBJECT_0);
	}

	// 논블로킹 완료 확인
	bool IsComplete() const
	{
		return m_count.load() <= 0;
	}

	// 이벤트 재사용 (count 재설정)
	void Reset(int32 count)
	{
		m_count.store(count);
		if (count > 0)
			::ResetEvent(m_event);
		else
			::SetEvent(m_event);
	}

private:
	Atomic<int32>	m_count;
	HANDLE			m_event = nullptr;
};
