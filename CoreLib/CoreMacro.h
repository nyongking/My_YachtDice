#pragma once

/*------------------
		CRASH
-------------------*/
#define TRIGGER_CRASH(cause)				\
{									\
	uint32* crash = nullptr;		\
	__analysis_assume(crash != nullptr); \
	*crash = 0xABCDE123;			\
}

#define ASSERT_TRIG_CRASH(expr)			\
{									\
	if (!(expr))					\
	{								\
		TRIGGER_CRASH("ASSERT_TRIG_CRASH");		\
		__analysis_assume(expr);	\
	}								\
}

/*-----------------
		LOCK
-----------------*/
#define USE_LOCKS(count)	Lock m_locks[count];
#define USE_LOCK			USE_LOCKS(1)
#define READ_LOCK_IDX(idx)	ReadLockGuard m_readLockGuard_##idx(m_locks[idx], typeid(this).name());
#define WRITE_LOCK_IDX(idx)	WriteLockGuard m_writeLockGuard_##idx(m_locks[idx] , typeid(this).name());
#define READ_LOCK			READ_LOCK_IDX(0)
#define WRITE_LOCK			WRITE_LOCK_IDX(0)