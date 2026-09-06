#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "Future.h"

class JobQueue : public std::enable_shared_from_this<JobQueue>
{
	friend class ThreadManager;
public:
	JobQueue() = default;
	virtual ~JobQueue() = default;

public:
	void DoAsync(bool pushOnly, CallbackType&& callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(callback)), pushOnly);
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(bool pushOnly, Ret(T::* Func)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());

		Push(ObjectPool<Job>::MakeShared(owner, Func, std::forward<Args>(args)...), pushOnly);
	}

	// 콜백의 반환값을 Future<T>로 받는 비동기 실행
	// 반환된 Future<T>로 결과를 나중에 조회하거나 블로킹 대기할 수 있다
	template<typename T>
	Future<T> DoAsyncWithResult(bool pushOnly, std::function<T()> callback)
	{
		auto [promise, future] = MakePromise<T>();

		// promise를 캡처해 콜백 결과를 SharedState에 저장
		Promise<T> capturedPromise = std::move(promise);
		Push(
			ObjectPool<Job>::MakeShared(
				[p = std::move(capturedPromise), cb = std::move(callback)]() mutable
				{
					try
					{
						p.SetValue(cb());
					}
					catch (...)
					{
						p.SetException(std::current_exception());
					}
				}
			),
			pushOnly
		);

		return future;
	}

	void DoGlobalTimer(uint64 tickAfter, CallbackType&& callback)
	{
		RefJob refJob = ObjectPool<Job>::MakeShared(std::move(callback));
		ReserveGlobalTimer(tickAfter, refJob);
	}

	template<typename T, typename Ret, typename... Args>
	void DoGlobalTimer(uint64 tickAfter, Ret(T::* Func)(Args...), Args... args)
	{
		std::shared_ptr<T> owner = std::static_pointer_cast<T>(shared_from_this());
		RefJob refJob = ObjectPool<Job>::MakeShared(owner, Func, std::forward<Args>(args)...);
		ReserveGlobalTimer(tickAfter, refJob);
	}

	void ClearJobs() { m_jobQueue.Clear(); }

public:
	virtual void Push(RefJob job, bool pushOnly);
	virtual void Execute();
	bool Empty() const { return m_jobCount.load() == 0; }

protected:
	LockQueue<RefJob>		m_jobQueue;
	Atomic<int32>			m_jobCount = 0;

private:
	void ReserveGlobalTimer(uint64 tickAfter, RefJob job);
};

