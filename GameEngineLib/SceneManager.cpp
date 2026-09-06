#include "GameEnginePch.h"
#include "SceneManager.h"
#include "CameraManager.h"
#include "UIManager.h"
#include "PhysicsManager.h"
#include "EngineGlobal.h"

namespace GameEngine
{
	void SceneManager::Update(float dt)
	{
		ApplyPendingScene();

		// Skip game logic while async resources are still loading
		if (m_loadState != SceneLoadState::None)
			return;

		if (GPhysicsManager)
			GPhysicsManager->Step(dt);

		if (m_currentScene)
		{
			m_currentScene->Update(dt);
			m_currentScene->LateUpdate(dt);
		}

		UIManager::GetInstance().SubmitUI(dt);
	}

	void SceneManager::Clear()
	{
		m_pendingScene.reset();

		if (GPhysicsManager)
			GPhysicsManager->Clear();

		m_currentScene.reset();
	}

	void SceneManager::ApplyPendingScene()
	{
		// Phase 1: kick off new scene (Awake)
		if (m_pendingScene)
		{
			if (GPhysicsManager)
				GPhysicsManager->Clear();

			CameraManager::GetInstance().SetMainCamera(nullptr);
			UIManager::GetInstance().ClearSceneCanvases();
			m_currentScene = std::move(m_pendingScene);
			m_currentScene->Awake();
			m_loadState = SceneLoadState::WaitingAsync;
		}

		// Phase 2: complete transition once async loads are done
		if (m_loadState == SceneLoadState::WaitingAsync
			&& m_currentScene && m_currentScene->IsAsyncLoadComplete())
		{
			m_currentScene->Start();
			m_loadState = SceneLoadState::None;
		}
	}
}
