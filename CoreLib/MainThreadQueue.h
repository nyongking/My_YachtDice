#pragma once
#include "Job.h"
#include "LockQueue.h"

class MainThreadQueue
{
public:
	MainThreadQueue();
	~MainThreadQueue();

public:
	void PushJob(CallbackType&& callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
	}

	template<typename T, typename Ret, typename... Args>
	void PushJob(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

	void Execute(uint32 size);
	void ExecuteAll();

private:
	void Push(RefJob job);

private:
	LockQueue<RefJob>	m_jobQueue;
};

