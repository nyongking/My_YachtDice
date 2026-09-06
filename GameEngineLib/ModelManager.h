#pragma once
#include "ResourceCache.h"
#include "Model.h"
#include "EngineGlobal.h"

namespace GameEngine
{
	class ModelManager : public ResourceCache<Model>
	{
#pragma region Singleton
	public:
		static ModelManager* GetInstance() { return GModelManager; }
		ModelManager() = default;
		ModelManager(const ModelManager&)            = delete;
		ModelManager& operator=(const ModelManager&) = delete;
#pragma endregion Singleton

	public:
		bool Initialize(RefCom<ID3D11Device> device);

		// 동기: .mymesh 파일 로드 후 캐시. 캐시 히트 시 기존 ptr 반환.
		Model* LoadSync(const std::string& key, const std::string& path);

		// 비동기: 워커 스레드에서 파일 로드 + GPU 버퍼 생성. 완료 시 콜백 호출.
		void LoadAsync(const std::string& key, const std::string& path,
					   std::function<void(Model*)> onComplete = nullptr);

		Model* Get(const std::string& key) const { return GetCached(key); }

	private:
		RefCom<ID3D11Device> m_device;
	};
}
