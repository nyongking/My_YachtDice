#pragma once

/*---------------------------------------
	EventSignal
	- Windows 이벤트 기반 단순 신호 메커니즘
	- Auto-reset 이벤트 사용 (대기자 1명 깨움)
	- CountdownEvent와 달리 카운터 없이 단순 Set/Wait
---------------------------------------*/

class EventSignal
{
public:
	EventSignal()
	{
		m_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		ASSERT_TRIG_CRASH(m_event != nullptr);
	}

	~EventSignal()
	{
		if (m_event != nullptr)
		{
			::CloseHandle(m_event);
			m_event = nullptr;
		}
	}

	// 복사/이동 금지
	EventSignal(const EventSignal&) = delete;
	EventSignal& operator=(const EventSignal&) = delete;
	EventSignal(EventSignal&&) = delete;
	EventSignal& operator=(EventSignal&&) = delete;

public:
	// 이벤트 신호 — 대기 중인 스레드 하나를 깨운다
	void Set()
	{
		::SetEvent(m_event);
	}

	// 이벤트가 신호 상태가 될 때까지 무한 대기
	void Wait()
	{
		::WaitForSingleObject(m_event, INFINITE);
	}

	// 타임아웃(밀리초) 지정 대기. true = 신호 수신, false = 타임아웃
	bool Wait(uint32 timeoutMs)
	{
		DWORD result = ::WaitForSingleObject(m_event, static_cast<DWORD>(timeoutMs));
		return (result == WAIT_OBJECT_0);
	}

	// 수동으로 비신호 상태로 초기화 (auto-reset이므로 보통 불필요)
	void Reset()
	{
		::ResetEvent(m_event);
	}

private:
	HANDLE m_event = nullptr;
};
