#pragma once
#include "Types.h"
#include "MemoryPool.h"
#include "AllocatorTrait.h"

template<typename T>
class ObjectPool
{
public:
	template<typename... Args>
	static T* Pop(Args&&... args)
	{
#ifdef _STOMP
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
		T* memory = static_cast<T*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
#elif _DEBUG
		T* memory = nullptr;

		if constexpr (AllocatorTrait<T>::UseStompAllocator)
		{
			MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
			memory = static_cast<T*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
		}
		else
		{
			memory = static_cast<T*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
		}
#else // release
		T* memory = static_cast<T*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
#endif
		new(memory)T(std::forward<Args>(args)...);
		return memory;
	}

	static void Push(T* obj)
	{
		obj->~T();
#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#elif _DEBUG
		if constexpr (AllocatorTrait<T>::UseStompAllocator)
		{
			StompAllocator::Release(MemoryHeader::DetachHeader(obj));
		}
		else
		{
			s_pool.Push(MemoryHeader::DetachHeader(obj));
		}
#else // release
		s_pool.Push(MemoryHeader::DetachHeader(obj));
#endif

	}

	template<typename... Args>
	static std::shared_ptr<T> MakeShared(Args&&... args)
	{
		std::shared_ptr<T> ptr = { Pop(std::forward<Args>(args)...), Push };
		return ptr;
	}

private:
	inline static int32		s_allocSize = sizeof(T) + sizeof(MemoryHeader);
	inline static MemoryPool	s_pool{ s_allocSize };
};