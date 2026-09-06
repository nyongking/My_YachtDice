#include "GameEnginePch.h"
#include "Timer.h"

namespace GameEngine
{
	Timer::Timer()
	{
		QueryPerformanceFrequency(&m_frequency);
		QueryPerformanceCounter(&m_prevTime);
		m_currTime = m_prevTime;
	}

	void Timer::Reset()
	{
		QueryPerformanceCounter(&m_prevTime);
		m_currTime = m_prevTime;
		m_totalTime = m_accumulator = m_sinceLastFrame = m_fpsTimer = 0.f;
		m_deltaTime  = 0.f;
		m_frameReady = false;
		m_frameCount = m_currentFPS = 0;
	}

	void Timer::Tick()
	{
		QueryPerformanceCounter(&m_currTime);

		const float rawDelta = static_cast<float>(
			m_currTime.QuadPart - m_prevTime.QuadPart)
			/ static_cast<float>(m_frequency.QuadPart);

		m_prevTime   = m_currTime;
		m_totalTime += rawDelta;

		// ── 프레임 준비 판단 ──
		if (m_targetInterval <= 0.f)
		{
			m_frameReady = true;
			m_deltaTime  = rawDelta;
		}
		else
		{
			m_sinceLastFrame += rawDelta;
			m_accumulator    += rawDelta;

			m_frameReady = (m_accumulator >= m_targetInterval);
			if (m_frameReady)
			{
				m_deltaTime      = m_sinceLastFrame; // 마지막 프레임 이후 실제 경과 시간
				m_sinceLastFrame = 0.f;
				m_accumulator   -= m_targetInterval; // 초과분 유지로 드리프트 방지
			}
		}

		// ── FPS 측정 (벽시계 기준 1초 단위 갱신) ──
		m_fpsTimer += rawDelta;
		if (m_frameReady)
			++m_frameCount;

		if (m_fpsTimer >= 1.0f)
		{
			m_currentFPS = m_frameCount;
			m_frameCount = 0;
			m_fpsTimer  -= 1.0f;
		}
	}

	void  Timer::SetTargetFPS(int fps)  { m_targetInterval = (fps > 0) ? (1.f / static_cast<float>(fps)) : 0.f; }
	bool  Timer::IsFrameReady() const   { return m_frameReady;  }
	float Timer::GetDeltaTime() const   { return m_deltaTime;   }
	float Timer::GetTotalTime() const   { return m_totalTime;   }
	int   Timer::GetFPS()       const   { return m_currentFPS;  }
}
