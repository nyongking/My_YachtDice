#pragma once
#include "JobQueue.h"

namespace GameEngine
{
    enum class ResourceState { NotLoaded, Loading, Ready, Failed };
    
    template<typename T>
    struct Entry
    {
        std::unique_ptr<T> resource;
        ResourceState      state = ResourceState::NotLoaded;
    };

    template<typename T>
    class ResourceCache : public JobQueue
    {
    public:
        ResourceState GetState(const std::string& key) const
        {
            ReadLockGuard lock(m_cacheLock, typeid(this).name());

            auto it = m_cache.find(key);

            if (it == m_cache.end())
                return ResourceState::Failed;

            return it->second.state;
        }

        void Clear()
        {
            WriteLockGuard lock(m_cacheLock, typeid(this).name());

            m_cache.clear();
        }

    protected:
        T* GetReserved(const std::string& key) const
        {
            ReadLockGuard lock(m_cacheLock, typeid(this).name());

            auto it = m_cache.find(key);

            if (it == m_cache.end() || it->second.state == ResourceState::Failed || it->second.state == ResourceState::NotLoaded)
                return nullptr;

            return it->second.resource.get();
        }

        T* GetCached(const std::string& key) const
        {
            ReadLockGuard lock(m_cacheLock, typeid(this).name());

            auto it = m_cache.find(key);

            if (it == m_cache.end() || it->second.state != ResourceState::Ready)
                return nullptr;

            return it->second.resource.get();
        }
      



        mutable Lock                                 m_cacheLock;
        std::unordered_map<std::string, Entry<T>>       m_cache;
    };
}
