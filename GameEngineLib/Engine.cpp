#include "GameEnginePch.h"
#include "Engine.h"

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

#include "ThreadManager.h"
#include "MainThreadQueue.h"

#include "Renderer.h"
#include "RenderDevice.h"
#include "RenderPipeline.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "AudioManager.h"

extern bool GRunning;

namespace
{
	uint64 g_workerTickMs = 25;
	uint64 g_workerWaitMs = 3;

	void DoWorkerJob()
	{
		while (GRunning)
		{
			DoGlobalWork(g_workerTickMs, g_workerWaitMs);
		}
	}
}

namespace GameEngine
{
	bool Engine::Init(const EngineConfig& config, const EngineCallbacks& callbacks)
	{
		m_callbacks = callbacks;

		// InitCore()가 이미 ThreadManager 생성자를 통해 메인 스레드에 고유
		// LThreadID(1)를 부여했다. 여기서 0으로 덮어쓰면 EMPTY_FLAG(=0)와
		// 겹쳐서 Lock::WriteLock()이 "아무도 안 잠근 상태"와 구분이 안 되고,
		// 그 틈에 워커 스레드의 ReadLock이 몰래 통과해 INVALID_UNLOCK_ORDER
		// 크래시로 이어진다 (Lock.cpp의 EMPTY_FLAG / WRITE_THREAD_MASK 참고).
		StartWorkers(config);

		timeBeginPeriod(1);
		m_timer.Reset();
		m_timer.SetTargetFPS(config.targetFPS);

		m_initialized = true;
		return true;
	}

	void Engine::Tick()
	{
		m_timer.Tick();
		if (!m_timer.IsFrameReady())
		{
			Sleep(1);
			return;
		}

		const float dt = m_timer.GetDeltaTime();

		InputManager::GetInstance().Update();

		GMainQueue->ExecuteAll();

		Render::RenderPipeline::GetInstance().BeginFrame();

		if (m_callbacks.onBeginFrame)
			m_callbacks.onBeginFrame();

		SceneManager::GetInstance().Update(dt);

		if (GAudioManager)
			GAudioManager->Update(dt);

		if (m_callbacks.onPostUpdate)
			m_callbacks.onPostUpdate();

		Render::Renderer::GetInstance().RenderBegin();
		Render::RenderPipeline::GetInstance().Execute(
			Render::RenderDevice::GetInstance().GetContext().Get());

		if (m_callbacks.onEndFrame)
			m_callbacks.onEndFrame();

		Render::Renderer::GetInstance().RenderEnd();
		Render::RenderPipeline::GetInstance().EndFrame();
	}

	void Engine::Shutdown()
	{
		timeEndPeriod(1);
		m_initialized = false;
	}

	void Engine::StartWorkers(const EngineConfig& config)
	{
		g_workerTickMs = config.workerTickMs;
		g_workerWaitMs = config.workerWaitMs;

		for (int i = 0; i < config.workerThreadCount; ++i)
		{
			GThreadManager->Launch([]() { DoWorkerJob(); });
		}
	}
}
