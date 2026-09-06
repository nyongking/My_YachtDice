#include "CorePch.h"
#include "MemoryPool.h"

MemoryPool::MemoryPool(int32 allocSize)
	: m_allocSize(allocSize)
{
	::InitializeSListHead(&m_header);
}

MemoryPool::~MemoryPool()
{
	while (MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&m_header)))
	{
		::_aligned_free(memory);
	}
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	ptr->m_allocSize = 0; // allocsize가 0이면 사용하지 않는 상태로 간주한다.

	// Pool에 메모리 반납
	::InterlockedPushEntrySList(&m_header, static_cast<SLIST_ENTRY*>(ptr));

	m_allocCount.fetch_sub(1);
	m_reserveCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&m_header));

	if (memory == nullptr) // 데이터가 없는 경우 새로 만든다.
	{
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(m_allocSize, SLIST_ALIGNMENT));
	}
	else // 여분이 있는 경우
	{
		ASSERT_TRIG_CRASH(memory->m_allocSize == 0);
		m_reserveCount.fetch_sub(1);
	}

	m_allocCount.fetch_add(1);

	return memory;
}
