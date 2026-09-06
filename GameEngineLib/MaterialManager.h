#pragma once
#include "ResourceCache.h"
#include "Material.h"
#include "EngineGlobal.h"

namespace GameEngine
{
	class MaterialManager : public ResourceCache<Render::Material>
	{
#pragma region Singleton
    public:
        static MaterialManager* GetInstance() { return GMaterialManager; }
        MaterialManager() = default;
        MaterialManager(const MaterialManager&) = delete;
        MaterialManager& operator=(const MaterialManager&) = delete;
#pragma endregion Singleton

    public:
        bool Initialize(RefCom<ID3D11Device> device);

        // 동기: shaderKey의 ShaderGroup이 Loading 상태면 Ready/Failed까지 블로킹. 캐시 원본의 Clone 반환
        std::unique_ptr<Render::Material> LoadSync(const std::string& key,
                                                   std::unique_ptr<Render::Material> mat,
                                                   const std::string& shaderKey);

        // 비동기: 워커 스레드에서 shader 대기 후 Initialize(). 콜백에 캐시 원본의 Clone 전달
        void LoadAsync(const std::string& key,
                       std::unique_ptr<Render::Material> mat,
                       const std::string& shaderKey,
                       std::function<void(std::unique_ptr<Render::Material>)> onComplete = nullptr);

        // 캐시 내용의 Clone을 반환 (호출자가 독립적 소유)
        std::unique_ptr<Render::Material> Get(const std::string& key) const;

    private:
        RefCom<ID3D11Device> m_device;
	};
}
