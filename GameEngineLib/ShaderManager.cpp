#include "GameEnginePch.h"
#include "ShaderManager.h"

namespace GameEngine
{
    bool ShaderManager::Initialize(RefCom<ID3D11Device> device, RefCom<ID3D11DeviceContext> context,
                                    const std::string& shaderBasePath)
    {
        if (!device.Get() || !context.Get())
            return false;

        m_device  = device;
        m_context = context;

        // ── 빌트인 셰이더 등록 ──
        if (!LoadSync("GBuffer",
                       shaderBasePath + "VSGBUFFER.hlsl",
                       shaderBasePath + "PSGBUFFER.hlsl"))
            return false;

        if (!LoadSync("DeferredLighting",
                       shaderBasePath + "VSFULLSCREEN.hlsl",
                       shaderBasePath + "PSLIGHTING.hlsl"))
            return false;

#ifdef _DEBUG
        if (!LoadSync("Wireframe",
                       shaderBasePath + "VSWIREFRAME.hlsl",
                       shaderBasePath + "PSWIREFRAME.hlsl"))
            return false;
#endif

        // UI 셰이더
        if (!LoadSync("UI",
                       shaderBasePath + "VSUI.hlsl",
                       shaderBasePath + "PSUI.hlsl"))
            return false;

        return true;
    }

    Render::ShaderGroup* ShaderManager::LoadSync(const std::string& key,
                                                  const std::string& vsPath,
                                                  const std::string& psPath)
    {
        {
            ReadLockGuard lock(m_cacheLock, typeid(this).name());

            auto it = m_cache.find(key);
            if (it != m_cache.end() && it->second.state == ResourceState::Ready)
                return it->second.resource.get();
        }

        Render::VertexShader* vs = LoadSyncVS(vsPath);
        Render::PixelShader*  ps = LoadSyncPS(psPath);

        if (nullptr == vs || nullptr == ps)
            return nullptr;

        WriteLockGuard lock(m_cacheLock, typeid(this).name());
        auto& entry    = m_cache[key];
        entry.resource = std::make_unique<Render::ShaderGroup>();
        entry.state    = ResourceState::Loading;

        if (vs && ps && entry.resource->Initialize(vs, ps, m_device.Get()))
        {
            entry.state = ResourceState::Ready;
            return entry.resource.get();
        }

        entry.state = ResourceState::Failed;
        return nullptr;
    }

    void ShaderManager::LoadAsync(const std::string& key,
                                   const std::string& vsPath,
                                   const std::string& psPath,
                                   std::function<void(Render::ShaderGroup*)> onComplete)
    {
        {
            WriteLockGuard lock(m_cacheLock, typeid(this).name());

            auto it = m_cache.find(key);
            if (it != m_cache.end() && it->second.state == ResourceState::Ready)
            {
                if (onComplete) onComplete(it->second.resource.get());
                return;
            }
            auto& entry    = m_cache[key];
            entry.resource = std::make_unique<Render::ShaderGroup>();
            entry.state    = ResourceState::Loading;
        }

        DoAsync(true, [this, key, vsPath, psPath, onComplete]()
        {
            Render::VertexShader* vs = LoadSyncVS(vsPath);
            Render::PixelShader*  ps = LoadSyncPS(psPath);

            if (nullptr == vs || nullptr == ps)
                return;

            Render::ShaderGroup* result = nullptr;
            {
                WriteLockGuard lock(m_cacheLock, typeid(this).name());
                auto& entry = m_cache[key];
                if (vs && ps && entry.resource->Initialize(vs, ps, m_device.Get()))
                {
                    entry.state = ResourceState::Ready;
                    result = entry.resource.get();
                }
                else
                {
                    entry.state = ResourceState::Failed;
                }
            }

            if (onComplete)
                onComplete(result);
        });
    }

    // ─── VS ─────────────────────────────────────────────────────────────────

    Render::VertexShader* ShaderManager::LoadSyncVS(const std::string& vsPath)
    {
        WriteLockGuard lock(m_vsLock, typeid(this).name());

        auto it = m_vs.find(vsPath);
        if (it != m_vs.end() && it->second.state == ResourceState::Ready)
            return it->second.resource.get();

        auto& entry    = m_vs[vsPath];
        entry.resource = std::make_unique<Render::VertexShader>();
        entry.state    = ResourceState::Loading;

        std::wstring wPath(vsPath.begin(), vsPath.end());
        if (!entry.resource->LoadFromFile(wPath) ||
            !entry.resource->CreateFromShaderBlob(m_device.Get()))
        {
            entry.state = ResourceState::Failed;
            return nullptr;
        }

        entry.state = ResourceState::Ready;
        return entry.resource.get();
    }

    void ShaderManager::LoadAsyncVS(const std::string& vsPath)
    {
        DoAsync(true, [this, vsPath]()
        {
            LoadSyncVS(vsPath);
        });
    }

    Render::VertexShader* ShaderManager::GetVS(const std::string& key)
    {
        ReadLockGuard lock(m_vsLock, typeid(this).name());

        auto it = m_vs.find(key);
        if (it == m_vs.end() || it->second.state != ResourceState::Ready)
            return nullptr;

        return it->second.resource.get();
    }

    // ─── PS ─────────────────────────────────────────────────────────────────

    Render::PixelShader* ShaderManager::LoadSyncPS(const std::string& psPath)
    {
        WriteLockGuard lock(m_psLock, typeid(this).name());

        auto it = m_ps.find(psPath);
        if (it != m_ps.end() && it->second.state == ResourceState::Ready)
            return it->second.resource.get();

        auto& entry    = m_ps[psPath];
        entry.resource = std::make_unique<Render::PixelShader>();
        entry.state    = ResourceState::Loading;

        std::wstring wPath(psPath.begin(), psPath.end());
        if (!entry.resource->LoadFromFile(wPath) ||
            !entry.resource->CreateFromShaderBlob(m_device.Get()))
        {
            entry.state = ResourceState::Failed;
            return nullptr;
        }

        entry.state = ResourceState::Ready;
        return entry.resource.get();
    }

    void ShaderManager::LoadAsyncPS(const std::string& psPath)
    {
        DoAsync(true, [this, psPath]()
        {
            LoadSyncPS(psPath);
        });
    }

    Render::PixelShader* ShaderManager::GetPS(const std::string& key)
    {
        ReadLockGuard lock(m_psLock, typeid(this).name());

        auto it = m_ps.find(key);
        if (it == m_ps.end() || it->second.state != ResourceState::Ready)
            return nullptr;

        return it->second.resource.get();
    }
}
