#include "GameEnginePch.h"
#include "AudioClip.h"

namespace GameEngine
{
	std::unique_ptr<AudioClip> AudioClip::LoadFromFile(
		DirectX::AudioEngine* engine, const std::wstring& path)
	{
		if (!engine)
			return nullptr;

		try
		{
			auto clip = std::make_unique<AudioClip>();
			clip->m_effect = std::make_unique<DirectX::SoundEffect>(engine, path.c_str());
			return clip;
		}
		catch (const std::exception&)
		{
			return nullptr;
		}
	}

	std::unique_ptr<DirectX::SoundEffectInstance> AudioClip::CreateInstance(
		DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags)
	{
		if (!m_effect)
			return nullptr;

		return m_effect->CreateInstance(flags);
	}

	void AudioClip::PlayOneShot(float volume, float pitch, float pan)
	{
		if (m_effect)
			m_effect->Play(volume, pitch, pan);
	}

	size_t AudioClip::GetDurationMS() const
	{
		if (!m_effect)
			return 0;

		return m_effect->GetSampleDurationMS();
	}
}
