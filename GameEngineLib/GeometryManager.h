#pragma once
#include "ResourceCache.h"
#include "Geometry.h"
#include "EngineGlobal.h"

namespace GameEngine
{
    class GeometryManager : public ResourceCache<Render::Geometry>
    {
#pragma region Singleton
    public:
        static GeometryManager* GetInstance() { return GGeometryManager; }
        GeometryManager() = default;
        GeometryManager(const GeometryManager&) = delete;
        GeometryManager& operator=(const GeometryManager&) = delete;
#pragma endregion Singleton

    public:
        bool Initialize(RefCom<ID3D11Device> device, RefCom<ID3D11DeviceContext> context);

        // 동기: 즉시 GPU 버퍼 생성. 캐시 히트 시 기존 ptr 반환.
        Render::Geometry* LoadSync(const std::string& key,
                                   std::unique_ptr<Render::Geometry> geo);

        // 비동기: 워커 스레드 풀에 잡 제출 (Device는 스레드 안전).
        void LoadAsync(const std::string& key,
                       std::unique_ptr<Render::Geometry> geo,
                       std::function<void(Render::Geometry*)> onComplete = nullptr);

        Render::Geometry* Get(const std::string& key) const { return GetCached(key); }

    private:
        RefCom<ID3D11Device>        m_device;
        RefCom<ID3D11DeviceContext> m_context;
    };
}
