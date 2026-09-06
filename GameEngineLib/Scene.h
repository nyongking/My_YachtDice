#pragma once

#include "GameObject.h"
#include "CountdownEvent.h"

namespace GameEngine
{
	class Scene
	{
	public:
		virtual ~Scene() = default;

		virtual void Awake() {}
		virtual void Start();


		virtual void Update(float dt);
		virtual void LateUpdate(float dt);

		GameObject* CreateGameObject(const std::string& name = "GameObject");
		void        DestroyGameObject(GameObject* go);

		// JSON 씬 파일 저장 / 로드
		void SaveToFile(const std::string& path) const;
		void LoadFromFile(const std::string& path);

		// Async load tracking: call BeginAsyncLoadTracking(N) in Awake(),
		// then Signal() the counter from each LoadAsync onComplete callback.
		// SceneManager defers Start() until IsAsyncLoadComplete() returns true.
		void BeginAsyncLoadTracking(int32 count);
		bool IsAsyncLoadComplete() const;

#ifdef _DEBUG
		const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_gameObjects; }
#endif

	protected:
		void FlushPendingDestroy();

		std::shared_ptr<CountdownEvent>          m_asyncLoadCounter;
		std::vector<std::unique_ptr<GameObject>> m_gameObjects;
		std::vector<GameObject*>                 m_pendingDestroy;
	};
}
