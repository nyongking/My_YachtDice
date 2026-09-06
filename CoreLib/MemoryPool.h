#pragma once

enum
{
	SLIST_ALIGNMENT = 16,
};


// 동일한 데이터 크기의 데이터끼리 관리한다
DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
	// [MemoryHeader][Real Data]

	MemoryHeader(int32 _size) : m_allocSize(_size) {}

	static void* AttachHeader(MemoryHeader* header, int32 size)
	{
		new(header)MemoryHeader(size); // placement new
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int32 m_allocSize;
};

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 allocSize);
	~MemoryPool();

	void			Push(MemoryHeader* ptr);
	MemoryHeader* Pop();

private:
	SLIST_HEADER m_header;
	int32 m_allocSize = 0; // 이 메모리풀이 담당할 메모리 크기
	Atomic<int32> m_allocCount = 0; // 할당된 개수
	Atomic<int32> m_reserveCount = 0; // 할당예약된 개수
};

