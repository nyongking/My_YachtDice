#include "GameEnginePch.h"
#include "GeometryManager.h"
#include "Quad.h"
#include "DebugGeometry.h"

namespace GameEngine
{
    bool GeometryManager::Initialize(RefCom<ID3D11Device> device, RefCom<ID3D11DeviceContext> context)
    {
        if (!device.Get() || !context.Get())
            return false;

        m_device  = device;
        m_context = context;

        LoadSync("Quad", std::make_unique<Quad>());

#ifdef _DEBUG
        // 와이어프레임 디버그 지오메트리 (초록 = 일반, 빨강 = trigger)
        float4 green = { 0.f, 1.f, 0.f, 1.f };
        float4 red   = { 1.f, 0.2f, 0.2f, 1.f };

        LoadSync("WireBox",           std::make_unique<WireBox>(green));
        LoadSync("WireSphere",        std::make_unique<WireSphere>(green));
        LoadSync("WirePlane",         std::make_unique<WirePlane>(green));
        LoadSync("WireBoxTrigger",    std::make_unique<WireBox>(red));
        LoadSync("WireSphereTrigger", std::make_unique<WireSphere>(red));
        LoadSync("WirePlaneTrigger",  std::make_unique<WirePlane>(red));
#endif

        return true;
    }


    Render::Geometry* GeometryManager::LoadSync(const std::string& key,
                                                 std::unique_ptr<Render::Geometry> geo)
    {
        WriteLockGuard lock(m_cacheLock, typeid(this).name());

        auto it = m_cache.find(key);
        if (it != m_cache.end() && it->second.state == ResourceState::Ready)
            return it->second.resource.get();

        auto& entry    = m_cache[key];
        entry.resource = std::move(geo);
        entry.state    = ResourceState::Loading;

        if (entry.resource->DefaultCreateBuffers(m_device.Get()))
            entry.state = ResourceState::Ready;
        else
            entry.state = ResourceState::Failed;

        return (entry.state == ResourceState::Ready) ? entry.resource.get() : nullptr;
    }

    void GeometryManager::LoadAsync(const std::string& key,
                                     std::unique_ptr<Render::Geometry> geo,
                                     std::function<void(Render::Geometry*)> onComplete)
    {
        Render::Geometry* rawPtr = geo.get();

        {
            WriteLockGuard lock(m_cacheLock, typeid(this).name());
            auto& entry    = m_cache[key];
            entry.resource = std::move(geo);
            entry.state    = ResourceState::Loading;
        }

        // pushOnly=true: 항상 워커 풀로 전달
        // ID3D11Device 리소스 생성은 스레드 안전 — 워커에서 직접 실행
        DoAsync(true, [this, key, rawPtr, onComplete]()
        {
            bool ok = rawPtr->DefaultCreateBuffers(m_device.Get());
            {
                WriteLockGuard lock(m_cacheLock, typeid(this).name());
                m_cache[key].state = ok ? ResourceState::Ready : ResourceState::Failed;
            }
            if (onComplete)
                onComplete(ok ? rawPtr : nullptr);
        });
    }
}
