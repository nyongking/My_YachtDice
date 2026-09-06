#include "CorePch.h"
#include "Lock.h"
#include "DeadLockProfiler.h"

const uint32 writePos = 16;

void Lock::WriteLock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 이미 동일 쓰레드가 소유권을 갖는다.
	const uint32 lockThreadID = (m_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (LThreadID == lockThreadID)
	{
		++m_writeCount;
		return;
	}

	// 소유권 경쟁

	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = ((LThreadID << writePos) & WRITE_THREAD_MASK);
	while (true) // spinlock
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = EMPTY_FLAG;
			if (m_lockFlag.compare_exchange_strong(OUT expected, desired)) // 읽기 중에는 실패
			{
				++m_writeCount;
				return;
			}
		}

		if (ACQUIRE_TIMEOUT_TICK <= ::GetTickCount64() - beginTick)
		{
			TRIGGER_CRASH("TIMEOUT"); // spinlock을 경합하는 시간이 너무 크다
		}

		std::this_thread::yield(); // 쓰레드 blocked
	}
}

void Lock::WriteUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// Readlock 해제 전에는 WriteUnlock불가능.
	if (0 != (m_lockFlag.load() & READ_COUNT_MASK))
	{
		TRIGGER_CRASH("INVALID_UNLOCK_ORDER")
	}

	const int32 lockCount = --m_writeCount;
	if (0 == lockCount)
		m_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 동일 쓰레드의 경우 소유하고 있으면 성공
	const uint32 lockThreadID = (m_lockFlag.load() & WRITE_THREAD_MASK) >> writePos;
	if (LThreadID == lockThreadID)
	{
		m_lockFlag.fetch_add(1);
		return;
	}

	// 아무도 소유하고 있지 않으면(WriteLock이 없는 경우) 경합해서 공유 카운트를 올린다.
	const int64 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; ++spinCount)
		{
			uint32 expected = (m_lockFlag.load() & READ_COUNT_MASK);
			if (m_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return;
		}

		if (ACQUIRE_TIMEOUT_TICK <= ::GetTickCount64() - beginTick)
		{
			TRIGGER_CRASH("TIMEOUT"); // spinlock을 경합하는 시간이 너무 크다
		}

		std::this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#ifdef _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	if (0 == (m_lockFlag.fetch_sub(1) & READ_COUNT_MASK))
	{
		TRIGGER_CRASH("INVALID_UNLOCK");
	}
}
