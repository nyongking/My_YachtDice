#pragma once
#include "Timer.h"
#include <functional>

namespace GameEngine
{
	struct EngineCallbacks
	{
		std::function<void()> onBeginFrame;  // after BeginFrame, before SceneManager::Update
		std::function<void()> onPostUpdate;  // after SceneManager::Update + Audio, before RenderBegin
		std::function<void()> onEndFrame;    // after Pipeline::Execute, before RenderEnd
	};

	struct EngineConfig
	{
		int    targetFPS        = 144;
		int    workerThreadCount = 5;
		uint64 workerTickMs     = 25;
		uint64 workerWaitMs     = 3;
	};

	class Engine
	{
	public:
		bool Init(const EngineConfig& config, const EngineCallbacks& callbacks = {});
		void Tick();
		void Shutdown();

		Timer& GetTimer() { return m_timer; }

	private:
		void StartWorkers(const EngineConfig& config);

		Timer           m_timer;
		EngineCallbacks m_callbacks;
		bool            m_initialized = false;
	};
}
