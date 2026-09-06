#pragma once

/* ----------------
   BaseAllocator
---------------- */

class  BaseAllocator
{
public:
	static void* Alloc(int32 size);
	static void	 Release(void* ptr);
};

/* ----------------
   StompAllocator
---------------- */

/* 메모리 오염을 확인하는 용도의 Allocator, debug 전용 */

class StompAllocator
{
public:
	enum { PAGE_SIZE = 0x1000, };

public:
	static void* Alloc(int32 size);
	static void	 Release(void* ptr);
};

/* ----------------
   PoolAllocator
---------------- */

/* 메모리 풀 전용 allocator */

class PoolAllocator
{
public:
	static void* Alloc(int32 size);
	static void	 Release(void* ptr);
};

/* ----------------
	STL-Allocator
---------------- */

template<typename T>
class STLAllocator
{
public:
	using value_type = T; // std::Allocator에 필요.

	STLAllocator() {}

	template<typename Other>
	STLAllocator(const STLAllocator<Other>&) {}

	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(PoolAllocator::Alloc(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		PoolAllocator::Release(ptr);
	}
};