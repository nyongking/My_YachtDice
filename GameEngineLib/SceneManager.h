#pragma once

#include "Scene.h"

namespace GameEngine
{
	enum class SceneLoadState { None, WaitingAsync };

	class SceneManager
	{
	public:
		static SceneManager& GetInstance()
		{
			static SceneManager instance;
			return instance;
		}

		template<typename T>
		void LoadScene()
		{
			static_assert(std::is_base_of<Scene, T>::value,
				"T must derive from Scene");
			m_pendingScene = std::make_unique<T>();
		}

		void   Update(float dt);
		void   Clear();
		Scene* GetCurrentScene() const { return m_currentScene.get(); }
		bool   IsLoading() const { return m_loadState == SceneLoadState::WaitingAsync; }

	private:
		SceneManager() = default;
		SceneManager(const SceneManager&)            = delete;
		SceneManager& operator=(const SceneManager&) = delete;

		void ApplyPendingScene();

		std::unique_ptr<Scene> m_currentScene;
		std::unique_ptr<Scene> m_pendingScene;
		SceneLoadState         m_loadState = SceneLoadState::None;
	};
}
