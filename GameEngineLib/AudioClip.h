#pragma once
#include <memory>
#include <string>
#include "DirectXTK/Audio.h"

namespace GameEngine
{
	class AudioClip
	{
	public:
		AudioClip() = default;
		~AudioClip() = default;

		AudioClip(const AudioClip&) = delete;
		AudioClip& operator=(const AudioClip&) = delete;

		// Load .wav file. Returns nullptr on failure.
		static std::unique_ptr<AudioClip> LoadFromFile(
			DirectX::AudioEngine* engine, const std::wstring& path);

		DirectX::SoundEffect* GetEffect() { return m_effect.get(); }

		// Create a controllable instance (for looping / 3D positioned sounds)
		std::unique_ptr<DirectX::SoundEffectInstance> CreateInstance(
			DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags = DirectX::SoundEffectInstance_Default);

		// Fire-and-forget one-shot
		void PlayOneShot(float volume = 1.f, float pitch = 0.f, float pan = 0.f);

		size_t GetDurationMS() const;

	private:
		std::unique_ptr<DirectX::SoundEffect> m_effect;
	};
}
