#include "GameEnginePch.h"
#include "MaterialManager.h"
#include "ShaderManager.h"

namespace GameEngine
{
    constexpr int MAX_WAIT_MS = 10000;

    bool MaterialManager::Initialize(RefCom<ID3D11Device> device)
    {
        if (!device.Get())
            return false;

        m_device = device;
        return true;
    }

    std::unique_ptr<Render::Material> MaterialManager::LoadSync(const std::string& key,
                                                               std::unique_ptr<Render::Material> mat,
                                                               const std::string& shaderKey)
    {
        // 1. Shader 상태 폴링 — 락 밖에서 처리
        auto& shaderMgr = *ShaderManager::GetInstance();
        int elapsed = 0;

        while (true)
        {
            ResourceState shaderState = shaderMgr.GetState(shaderKey);

            if (shaderState == ResourceState::Ready)
                break;
            if (shaderState == ResourceState::Failed || shaderState == ResourceState::NotLoaded)
                return nullptr;

            Sleep(10);
            elapsed += 10;
            if (elapsed >= MAX_WAIT_MS)
                return nullptr;
        }

        Render::ShaderGroup* sg = shaderMgr.Get(shaderKey);
        if (!sg || !m_device)
            return nullptr;

        // 2. 캐시 히트 확인 + 등록 + Initialize
        Render::Material* rawPtr = mat.get();

        WriteLockGuard lock(m_cacheLock, typeid(this).name());

        auto it = m_cache.find(key);
        if (it != m_cache.end() && it->second.state == ResourceState::Ready)
            return it->second.resource->Clone();

        auto& entry    = m_cache[key];
        entry.resource = std::move(mat);
        entry.state    = ResourceState::Loading;

        if (rawPtr->Initialize(sg, m_device.Get()))
        {
            entry.state = ResourceState::Ready;
            return rawPtr->Clone();
        }

        entry.state = ResourceState::Failed;
        return nullptr;
    }

    void MaterialManager::LoadAsync(const std::string& key,
                                    std::unique_ptr<Render::Material> mat,
                                    const std::string& shaderKey,
                                    std::function<void(std::unique_ptr<Render::Material>)> onComplete)
    {
        Render::Material* rawPtr = mat.get();

        // 1. 캐시 히트 확인 + 선점
        {
            WriteLockGuard lock(m_cacheLock, typeid(this).name());

            auto it = m_cache.find(key);
            if (it != m_cache.end() && it->second.state == ResourceState::Ready)
            {
                if (onComplete) onComplete(it->second.resource->Clone());
                return;
            }

            auto& entry    = m_cache[key];
            entry.resource = std::move(mat);
            entry.state    = ResourceState::Loading;
        }

        // 2. 워커 스레드: shader 대기 → Initialize
        DoAsync(true, [this, key, shaderKey, rawPtr, onComplete]()
        {
            auto& shaderMgr = *ShaderManager::GetInstance();

            int elapsed = 0;

            while (true)
            {
                ResourceState shaderState = shaderMgr.GetState(shaderKey);
                if (shaderState != ResourceState::Loading)
                    break;

                Sleep(10);
                elapsed += 10;
                if (elapsed >= MAX_WAIT_MS)
                    break;
            }

            Render::ShaderGroup* sg = shaderMgr.Get(shaderKey);
            std::unique_ptr<Render::Material> result = nullptr;

            {
                WriteLockGuard lock(m_cacheLock, typeid(this).name());
                auto& entry = m_cache[key];
                if (sg && m_device && rawPtr->Initialize(sg, m_device.Get()))
                {
                    entry.state = ResourceState::Ready;
                    result = rawPtr->Clone();
                }
                else
                {
                    entry.state = ResourceState::Failed;
                }
            }

            if (onComplete) onComplete(std::move(result));
        });
    }

    std::unique_ptr<Render::Material> MaterialManager::Get(const std::string& key) const
    {
        Render::Material* cached = GetCached(key);
        if (!cached)
            return nullptr;

        return cached->Clone();
    }
}
