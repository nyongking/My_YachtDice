#pragma once
#include <Windows.h>

namespace GameEngine
{
	class Timer
	{
	public:
		Timer();

		void Reset();               // 게임 시작 시 1회 호출
		void Tick();                // 매 루프 최상단에서 호출

		void SetTargetFPS(int fps); // 0 = 무제한
		bool IsFrameReady() const;  // 이번 루프에서 Update/Render 할지 여부

		float GetDeltaTime() const; // 프레임 dt (초)
		float GetTotalTime() const; // 누적 경과 시간 (초)
		int   GetFPS()       const; // 측정된 실제 FPS

	private:
		LARGE_INTEGER m_frequency  = {};
		LARGE_INTEGER m_prevTime   = {};
		LARGE_INTEGER m_currTime   = {};

		float m_deltaTime      = 0.f;   // 프레임 dt (직전 frame-ready 이후 경과 시간)
		float m_totalTime      = 0.f;

		float m_targetInterval = 0.f;   // 1.f/fps (0 = 무제한)
		float m_accumulator    = 0.f;
		float m_sinceLastFrame = 0.f;   // 마지막 frame-ready 이후 누적 raw 시간
		bool  m_frameReady     = false;

		// FPS 측정
		int   m_frameCount  = 0;
		float m_fpsTimer    = 0.f;
		int   m_currentFPS  = 0;
	};
}
