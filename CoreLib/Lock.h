#pragma once

/* -----------------
Read-write spinlock
-------------------- */

/*
앞 16비트 : write 중인 threadID
뒤 16비트 : read 중인 thread 개수
*/

/*
	표준 mutex와 차별화되는 점
	1. 재귀적으로 LOCK을 잡는다.
	2. 데이터를 쓸 때는 상호배타적, 읽을 때는 상관없다.
*/

class Lock
{
	enum : uint32_t
	{
		ACQUIRE_TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF0000,
		READ_COUNT_MASK = 0x0000FFFF,
		EMPTY_FLAG = 0x00000000,
	};

public:
	void WriteLock(const char* name);
	void WriteUnlock(const char* name);
	void ReadLock(const char* name);
	void ReadUnlock(const char* name);

private:
	std::atomic<uint32_t> m_lockFlag = EMPTY_FLAG;;
	uint16_t m_writeCount = 0;

};

/* -----------------
	RAII for Lock
------------------- */

class ReadLockGuard
{
public:
	ReadLockGuard(Lock& lock, const char* name) : m_lock(lock), m_name(name) { m_lock.ReadLock(name); }
	~ReadLockGuard() { m_lock.ReadUnlock(m_name); }

private:
	Lock& m_lock;
	const char* m_name;
};

class WriteLockGuard
{
public:
	WriteLockGuard(Lock& lock, const char* name) : m_lock(lock), m_name(name) { m_lock.WriteLock(name); }
	~WriteLockGuard() { m_lock.WriteUnlock(m_name); }

private:
	Lock& m_lock;
	const char* m_name;
};