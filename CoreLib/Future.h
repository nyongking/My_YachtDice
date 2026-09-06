#pragma once
#include <exception>
#include <utility>

/*---------------------------------------
	SharedState<T>
	- Promise와 Future가 공유하는 내부 상태
	- Windows Event 기반 블로킹 대기
---------------------------------------*/

template<typename T>
struct SharedState
{
	SharedState()
	{
		m_event = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
		ASSERT_TRIG_CRASH(m_event != nullptr);
	}

	~SharedState()
	{
		if (m_event != nullptr)
		{
			::CloseHandle(m_event);
			m_event = nullptr;
		}
	}

	SharedState(const SharedState&) = delete;
	SharedState& operator=(const SharedState&) = delete;

	// 값 설정 (Promise 측에서 호출)
	void SetValue(T value)
	{
		ASSERT_TRIG_CRASH(!m_ready.load());
		m_value = std::move(value);
		m_ready.store(true);
		::SetEvent(m_event);

		CallbackType cb;
		{
			WRITE_LOCK;
			cb = std::move(m_continuation);
		}
		if (cb)
			cb();
	}

	// 예외 설정 (Promise 측에서 호출)
	void SetException(std::exception_ptr ex)
	{
		ASSERT_TRIG_CRASH(!m_ready.load());
		m_exception = ex;
		m_ready.store(true);
		::SetEvent(m_event);

		CallbackType cb;
		{
			WRITE_LOCK;
			cb = std::move(m_continuation);
		}
		if (cb)
			cb();
	}

	// 블로킹 대기 후 값 반환 (예외가 있으면 rethrow)
	T& Get()
	{
		if (!m_ready.load())
			::WaitForSingleObject(m_event, INFINITE);

		if (m_exception)
			std::rethrow_exception(m_exception);

		return m_value;
	}

	// 논블로킹 시도. 준비되었으면 out에 복사 후 true 반환
	bool TryGet(T& out)
	{
		if (!m_ready.load())
			return false;

		if (m_exception)
			std::rethrow_exception(m_exception);

		out = m_value;
		return true;
	}

	// 완료 시 실행할 콜백 등록. 이미 완료 상태라면 즉시 실행
	void SetContinuation(CallbackType cb)
	{
		bool alreadyReady = false;
		{
			WRITE_LOCK;
			if (m_ready.load())
				alreadyReady = true;
			else
				m_continuation = std::move(cb);
		}
		if (alreadyReady && cb)
			cb();
	}

	bool IsReady() const { return m_ready.load(); }

	USE_LOCK;

private:
	Atomic<bool>		m_ready = false;
	T					m_value = {};
	std::exception_ptr	m_exception;
	CallbackType		m_continuation;
	HANDLE				m_event = nullptr;
};


/*---------------------------------------
	Future<T>
	- 비동기 결과를 소비하는 쪽 핸들
---------------------------------------*/

template<typename T>
class Future
{
public:
	Future() = default;
	explicit Future(Ref<SharedState<T>> state) : m_state(std::move(state)) {}

	bool IsReady() const
	{
		ASSERT_TRIG_CRASH(m_state != nullptr);
		return m_state->IsReady();
	}

	// 블로킹 대기 후 값 반환
	T& Get()
	{
		ASSERT_TRIG_CRASH(m_state != nullptr);
		return m_state->Get();
	}

	// 논블로킹 시도
	bool TryGet(T& out)
	{
		ASSERT_TRIG_CRASH(m_state != nullptr);
		return m_state->TryGet(out);
	}

	// 완료 시 실행할 콜백 등록 (void() 시그니처)
	void Then(CallbackType callback)
	{
		ASSERT_TRIG_CRASH(m_state != nullptr);
		m_state->SetContinuation(std::move(callback));
	}

	bool IsValid() const { return m_state != nullptr; }

private:
	Ref<SharedState<T>> m_state;
};


/*---------------------------------------
	Promise<T>
	- 비동기 결과를 생산하는 쪽 핸들
---------------------------------------*/

template<typename T>
class Promise
{
public:
	Promise() = default;
	explicit Promise(Ref<SharedState<T>> state) : m_state(std::move(state)) {}

	void SetValue(T value)
	{
		ASSERT_TRIG_CRASH(m_state != nullptr);
		m_state->SetValue(std::move(value));
	}

	void SetException(std::exception_ptr ex)
	{
		ASSERT_TRIG_CRASH(m_state != nullptr);
		m_state->SetException(ex);
	}

	bool IsValid() const { return m_state != nullptr; }

private:
	Ref<SharedState<T>> m_state;
};


/*---------------------------------------
	MakePromise<T>
	- Promise와 Future 쌍을 생성
	- pair.first = Promise<T>, pair.second = Future<T>
---------------------------------------*/

template<typename T>
std::pair<Promise<T>, Future<T>> MakePromise()
{
	auto state = std::make_shared<SharedState<T>>();
	return { Promise<T>(state), Future<T>(state) };
}
