#include "GameEnginePch.h"
#include "TextureManager.h"

namespace GameEngine
{
	bool TextureManager::Initialize(RefCom<ID3D11Device> device, RefCom<ID3D11DeviceContext> context)
	{
		if (!device.Get() || !context.Get())
			return false;

		m_device  = device;
		m_context = context;

		// 1x1 흰색 기본 텍스처 등록 ("White")
		{
			uint32_t white = 0xFFFFFFFF;
			D3D11_TEXTURE2D_DESC td = {};
			td.Width = 1; td.Height = 1; td.MipLevels = 1; td.ArraySize = 1;
			td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_IMMUTABLE;
			td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = &white; sd.SysMemPitch = 4;

			RefCom<ID3D11Texture2D> tex2D;
			RefCom<ID3D11ShaderResourceView> srv;
			if (SUCCEEDED(device->CreateTexture2D(&td, &sd, tex2D.GetAddressOf())) &&
				SUCCEEDED(device->CreateShaderResourceView(tex2D.Get(), nullptr, srv.GetAddressOf())))
			{
				auto tex = std::make_unique<Render::Texture>();
				tex->InitFromRaw(tex2D.Get(), srv.Get(), 1, 1);

				WriteLockGuard lock(m_cacheLock, typeid(this).name());
				auto& entry    = m_cache["White"];
				entry.resource = std::move(tex);
				entry.state    = ResourceState::Ready;
			}
		}

		return true;
	}

	Render::Texture* TextureManager::LoadSync(const std::string& key, const std::wstring& path)
	{
		WriteLockGuard lock(m_cacheLock, typeid(this).name());

		auto it = m_cache.find(key);
		if (it != m_cache.end() && it->second.state == ResourceState::Ready)
			return it->second.resource.get();

		auto& entry = m_cache[key]; // cache 등록
		entry.state = ResourceState::Loading;

		entry.resource = Render::Texture::LoadFromFile(m_device.Get(), path);

		entry.state = entry.resource ? ResourceState::Ready : ResourceState::Failed;

		return (entry.state == ResourceState::Ready) ? entry.resource.get() : nullptr;
	}

	void TextureManager::LoadAsync(const std::string& key, const std::wstring& path,
	                               std::function<void(Render::Texture*)> onComplete)
	{
		{
			WriteLockGuard lock(m_cacheLock, typeid(this).name());
			auto it = m_cache.find(key);
			if (it != m_cache.end() && it->second.state == ResourceState::Ready)
			{
				if (onComplete)
					onComplete(it->second.resource.get());
				return; // 이미 존재하는 상태면 Return;
			}
			m_cache[key].state = ResourceState::Loading;
		}

		// ID3D11Device 리소스 생성은 스레드 안전 — 워커에서 직접 실행
		DoAsync(true, [this, key, path, onComplete]()
		{
			auto tex = Render::Texture::LoadFromFile(m_device.Get(), path);
			bool ok  = (tex != nullptr);
			{
				WriteLockGuard lock(m_cacheLock, typeid(this).name());
				auto& entry = m_cache[key]; // cache 등록
				entry.resource = std::move(tex);
				entry.state    = ok ? ResourceState::Ready : ResourceState::Failed;
			}
			if (onComplete)
				onComplete(ok ? m_cache[key].resource.get() : nullptr);
		});
	}
}
