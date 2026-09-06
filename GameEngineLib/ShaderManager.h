#pragma once
#include "ResourceCache.h"
#include "ShaderGroup.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "EngineGlobal.h"

namespace GameEngine
{
    class ShaderManager : public ResourceCache<Render::ShaderGroup>
    {
#pragma region Singleton
    public:
        static ShaderManager* GetInstance() { return GShaderManager; }
        ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;
#pragma endregion Singleton

    public:
        bool Initialize(RefCom<ID3D11Device> device, RefCom<ID3D11DeviceContext> context,
                        const std::string& shaderBasePath);

        Render::ShaderGroup* LoadSync(const std::string& key, const std::string& vsPath, const std::string& psPath);

        void LoadAsync(const std::string& key,
            const std::string& vsPath, const std::string& psPath,
            std::function<void(Render::ShaderGroup*)> onComplete = nullptr);

        Render::ShaderGroup* Get(const std::string& key) const { return GetCached(key); }

    private:
        Render::VertexShader* LoadSyncVS(const std::string& vsPath);
        Render::PixelShader* LoadSyncPS(const std::string& psPath);
        void LoadAsyncVS(const std::string& vsPath);
        void LoadAsyncPS(const std::string& psPath);

        Render::VertexShader* GetVS(const std::string& key);
        Render::PixelShader*  GetPS(const std::string& key);

    private:
        RefCom<ID3D11Device>        m_device;
        RefCom<ID3D11DeviceContext> m_context;

        Lock m_vsLock;
        Lock m_psLock;

        std::unordered_map<std::string, Entry<Render::VertexShader>> m_vs;
        std::unordered_map<std::string, Entry<Render::PixelShader>>  m_ps;
    };
}
