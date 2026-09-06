#pragma once
#include "Allocator.h"

class MemoryPool;

class Memory
{
	enum
	{
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	std::vector<MemoryPool*> m_pools;
	MemoryPool* m_poolTable[MAX_ALLOC_SIZE + 1]; // 메모리 크기가 몇이냐에 따라 MemoryPool이 설정됨
};

template<typename Type, typename... Args> // 가변인자
Type* exnew(Args&&... args)
{
	// 메모리 크기만큼 할당
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));

	// placement new, 이미 할당된 구역에 생성자만 호출
	new(memory)Type(std::forward<Args>(args)...);

	return memory;
}

template<typename Type>
void exdelete(Type* ptr)
{
	ptr->~Type();
	//exrelease::Release(ptr);
	PoolAllocator::Release(ptr);
}

template<typename T, typename... Args>
std::shared_ptr<T> MakeShared(Args&&... args)
{
	return std::shared_ptr<T> {exnew<T>(std::forward<Args>(args)...), exdelete<T>};
}
