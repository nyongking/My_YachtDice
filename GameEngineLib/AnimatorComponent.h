#pragma once

#include "Component.h"
#include "KeyframeAnimation.h"

namespace GameEngine
{
	class Transform;

	class AnimatorComponent : public Component
	{
	public:
		// Play animation by name. Resets and starts from beginning.
		void Play(const std::string& name);

		// Blend from current pos/rot into target animation over blendDuration seconds.
		void Transition(const std::string& name, float blendDuration);

		// Stop current animation
		void Stop();

		bool IsFinished() const;
		bool IsPlaying()  const;

		const std::string& GetCurrentName() const { return m_currentName; }

	public:
		void Start() override;
		void Update(float dt) override;

		std::string GetTypeName()            const override { return "AnimatorComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

	private:
		Transform*   m_transform = nullptr;

		std::unordered_map<std::string, KeyframeAnimation> m_animations;
		KeyframeAnimation* m_current     = nullptr;
		std::string        m_currentName;
		std::string        m_startAnimName;

		// Transition blend
		bool    m_blending       = false;
		float   m_blendElapsed   = 0.f;
		float   m_blendDuration  = 0.f;
		float3  m_blendStartPos   = {};
		float3  m_blendStartRot   = {};
		float3  m_blendStartScale = { 1.f, 1.f, 1.f };
	};
}
