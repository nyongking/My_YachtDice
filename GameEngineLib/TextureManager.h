#pragma once
#include "ResourceCache.h"
#include "Texture.h"
#include "EngineGlobal.h"

namespace GameEngine
{
	class TextureManager : public ResourceCache<Render::Texture>
	{
#pragma region Singleton
	public:
		static TextureManager* GetInstance() { return GTextureManager; }
		TextureManager() = default;
		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;
#pragma endregion Singleton

	public:
		bool Initialize(RefCom<ID3D11Device> device, RefCom<ID3D11DeviceContext> context);

		// 동기: 메인 스레드에서 즉시 로드·캐시.
		Render::Texture* LoadSync(const std::string& key, const std::wstring& path);

		// 비동기: 워커 스레드에서 로드. ID3D11Device는 스레드 안전.
		void LoadAsync(const std::string& key, const std::wstring& path,
		               std::function<void(Render::Texture*)> onComplete = nullptr);

		Render::Texture* Get(const std::string& key) const { return GetCached(key); }

	private:
		RefCom<ID3D11Device>        m_device;
		RefCom<ID3D11DeviceContext> m_context;
	};
}
