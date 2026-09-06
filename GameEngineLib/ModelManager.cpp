#include "GameEnginePch.h"
#include "ModelManager.h"

namespace GameEngine
{
	bool ModelManager::Initialize(RefCom<ID3D11Device> device)
	{
		if (!device.Get())
			return false;

		m_device = device;
		return true;
	}

	Model* ModelManager::LoadSync(const std::string& key, const std::string& path)
	{
		if (!m_device)
			return nullptr;

		// 캐시 히트
		{
			ReadLockGuard lock(m_cacheLock, typeid(this).name());
			auto it = m_cache.find(key);
			if (it != m_cache.end() && it->second.state == ResourceState::Ready)
				return it->second.resource.get();
		}

		// 선점 등록
		{
			WriteLockGuard lock(m_cacheLock, typeid(this).name());
			auto& entry  = m_cache[key];
			entry.resource = std::make_unique<Model>();
			entry.state    = ResourceState::Loading;
		}

		// 로드 (락 밖에서)
		Model* rawPtr = nullptr;
		{
			ReadLockGuard lock(m_cacheLock, typeid(this).name());
			rawPtr = m_cache[key].resource.get();
		}

		bool ok = rawPtr->Load(path, m_device.Get());

		{
			WriteLockGuard lock(m_cacheLock, typeid(this).name());
			m_cache[key].state = ok ? ResourceState::Ready : ResourceState::Failed;
		}

		return ok ? rawPtr : nullptr;
	}

	void ModelManager::LoadAsync(const std::string& key, const std::string& path,
								 std::function<void(Model*)> onComplete)
	{
		if (!m_device)
		{
			if (onComplete) onComplete(nullptr);
			return;
		}

		// 캐시 히트
		{
			ReadLockGuard lock(m_cacheLock, typeid(this).name());
			auto it = m_cache.find(key);
			if (it != m_cache.end() && it->second.state == ResourceState::Ready)
			{
				if (onComplete) onComplete(it->second.resource.get());
				return;
			}
		}

		// 선점 등록
		Model* rawPtr = nullptr;
		{
			WriteLockGuard lock(m_cacheLock, typeid(this).name());
			auto& entry    = m_cache[key];
			entry.resource = std::make_unique<Model>();
			entry.state    = ResourceState::Loading;
			rawPtr         = entry.resource.get();
		}

		// 워커 스레드: 파일 I/O + GPU 버퍼 생성 (Device::CreateBuffer는 스레드 안전)
		DoAsync(true, [this, key, path, rawPtr, onComplete]()
		{
			bool ok = rawPtr->Load(path, m_device.Get());
			{
				WriteLockGuard lock(m_cacheLock, typeid(this).name());
				m_cache[key].state = ok ? ResourceState::Ready : ResourceState::Failed;
			}
			if (onComplete) onComplete(ok ? rawPtr : nullptr);
		});
	}
}
