#include "GameEnginePch.h"
#include "AudioManager.h"

namespace GameEngine
{
	bool AudioManager::Initialize()
	{
		try
		{
			DirectX::AUDIO_ENGINE_FLAGS flags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
			flags |= DirectX::AudioEngine_Debug;
#endif
			m_audioEngine = std::make_unique<DirectX::AudioEngine>(flags);
		}
		catch (const std::exception&)
		{
			// Audio device not available -- run in silent mode
			return true;
		}

		return true;
	}

	void AudioManager::Update(float dt)
	{
		if (!m_audioEngine)
			return;

		// Per-frame XAudio2 housekeeping
		if (!m_audioEngine->Update())
		{
			// Audio device lost -- try to recover
			if (m_audioEngine->IsCriticalError())
				m_audioEngine->Reset();
		}

		UpdateBGMFade(dt);
	}

	void AudioManager::Shutdown()
	{
		// Stop all playback instances before destroying clips
		if (m_bgmInstance)     { m_bgmInstance->Stop(true);     m_bgmInstance.reset(); }
		if (m_bgmFadeOutInstance) { m_bgmFadeOutInstance->Stop(true); m_bgmFadeOutInstance.reset(); }

		// Destroy all cached AudioClips (SoundEffects) while AudioEngine is still alive
		{
			WriteLockGuard lock(m_cacheLock, typeid(this).name());
			m_cache.clear();
		}

		// AudioEngine destructor handles XAudio2 cleanup internally
		m_audioEngine.reset();
	}

	// ---- Resource loading ----

	AudioClip* AudioManager::LoadSync(const std::string& key, const std::wstring& path)
	{
		WriteLockGuard lock(m_cacheLock, typeid(this).name());

		auto it = m_cache.find(key);
		if (it != m_cache.end() && it->second.state == ResourceState::Ready)
			return it->second.resource.get();

		auto& entry = m_cache[key];
		entry.state = ResourceState::Loading;

		entry.resource = AudioClip::LoadFromFile(m_audioEngine.get(), path);
		entry.state = entry.resource ? ResourceState::Ready : ResourceState::Failed;

		return (entry.state == ResourceState::Ready) ? entry.resource.get() : nullptr;
	}

	void AudioManager::LoadAsync(const std::string& key, const std::wstring& path,
	                             std::function<void(AudioClip*)> onComplete)
	{
		{
			WriteLockGuard lock(m_cacheLock, typeid(this).name());
			auto it = m_cache.find(key);
			if (it != m_cache.end() && it->second.state == ResourceState::Ready)
			{
				if (onComplete)
					onComplete(it->second.resource.get());
				return;
			}
			m_cache[key].state = ResourceState::Loading;
		}

		// SoundEffect constructor reads WAV data (thread-safe for file I/O).
		// XAudio2 voice creation happens later at Play/CreateInstance time.
		DoAsync(true, [this, key, path, onComplete]()
		{
			auto clip = AudioClip::LoadFromFile(m_audioEngine.get(), path);
			bool ok = (clip != nullptr);
			{
				WriteLockGuard lock(m_cacheLock, typeid(this).name());
				auto& entry    = m_cache[key];
				entry.resource = std::move(clip);
				entry.state    = ok ? ResourceState::Ready : ResourceState::Failed;
			}
			if (onComplete)
				onComplete(ok ? m_cache[key].resource.get() : nullptr);
		});
	}

	// ---- Volume control ----

	void AudioManager::SetMasterVolume(float v)
	{
		m_masterVolume = std::clamp(v, 0.f, 1.f);
		if (m_audioEngine)
			m_audioEngine->SetMasterVolume(m_masterVolume);
	}

	void AudioManager::SetBGMVolume(float v)
	{
		m_bgmVolume = std::clamp(v, 0.f, 1.f);
		ApplyBGMVolume();
	}

	// ---- BGM playback ----

	void AudioManager::PlayBGM(const std::string& clipKey, float fadeInSec)
	{
		if (clipKey == m_currentBGMKey && m_bgmInstance &&
			m_bgmInstance->GetState() == DirectX::PLAYING)
			return;

		AudioClip* clip = Get(clipKey);
		if (!clip)
			return;

		// Fade out current BGM if present
		if (m_bgmInstance && m_bgmInstance->GetState() != DirectX::STOPPED)
		{
			m_bgmFadeOutInstance = std::move(m_bgmInstance);
			m_bgmFadeOutTimer   = 0.f;
			m_bgmFadeOutDuration = (fadeInSec > 0.f) ? fadeInSec : 0.3f;
			m_bgmFadingOut = true;
		}

		// Create new BGM instance
		m_bgmInstance = clip->CreateInstance();
		if (!m_bgmInstance)
			return;

		m_currentBGMKey = clipKey;

		if (fadeInSec > 0.f)
		{
			m_bgmInstance->SetVolume(0.f);
			m_bgmFadeTimer    = 0.f;
			m_bgmFadeDuration = fadeInSec;
			m_bgmFadingIn     = true;
		}
		else
		{
			m_bgmInstance->SetVolume(m_bgmVolume);
			m_bgmFadingIn = false;
		}

		m_bgmInstance->Play(true); // loop
	}

	void AudioManager::StopBGM(float fadeOutSec)
	{
		if (!m_bgmInstance)
			return;

		if (fadeOutSec > 0.f)
		{
			m_bgmFadeOutInstance = std::move(m_bgmInstance);
			m_bgmFadeOutTimer   = 0.f;
			m_bgmFadeOutDuration = fadeOutSec;
			m_bgmFadingOut = true;
			m_bgmFadingIn  = false;
		}
		else
		{
			m_bgmInstance->Stop();
			m_bgmInstance.reset();
		}

		m_currentBGMKey.clear();
	}

	void AudioManager::PauseBGM()
	{
		if (m_bgmInstance && m_bgmInstance->GetState() == DirectX::PLAYING)
			m_bgmInstance->Pause();
	}

	void AudioManager::ResumeBGM()
	{
		if (m_bgmInstance && m_bgmInstance->GetState() == DirectX::PAUSED)
			m_bgmInstance->Resume();
	}

	bool AudioManager::IsBGMPlaying() const
	{
		return m_bgmInstance && m_bgmInstance->GetState() == DirectX::PLAYING;
	}

	// ---- SFX convenience ----

	void AudioManager::PlaySFX(const std::string& clipKey,
	                           float volume, float pitch, float pan)
	{
		AudioClip* clip = Get(clipKey);
		if (!clip)
			return;

		clip->PlayOneShot(volume * m_sfxVolume, pitch, pan);
	}

	// ---- 3D audio listener ----

	void AudioManager::SetActiveListener(const DirectX::AudioListener& listener)
	{
		m_listener    = listener;
		m_hasListener = true;
	}

	// ---- Internal helpers ----

	void AudioManager::UpdateBGMFade(float dt)
	{
		// Fade-in
		if (m_bgmFadingIn && m_bgmInstance)
		{
			m_bgmFadeTimer += dt;
			float t = std::clamp(m_bgmFadeTimer / m_bgmFadeDuration, 0.f, 1.f);
			m_bgmInstance->SetVolume(t * m_bgmVolume);

			if (t >= 1.f)
				m_bgmFadingIn = false;
		}

		// Fade-out (old BGM during crossfade)
		if (m_bgmFadingOut && m_bgmFadeOutInstance)
		{
			m_bgmFadeOutTimer += dt;
			float t = std::clamp(m_bgmFadeOutTimer / m_bgmFadeOutDuration, 0.f, 1.f);
			m_bgmFadeOutInstance->SetVolume((1.f - t) * m_bgmVolume);

			if (t >= 1.f)
			{
				m_bgmFadeOutInstance->Stop();
				m_bgmFadeOutInstance.reset();
				m_bgmFadingOut = false;
			}
		}
	}

	void AudioManager::ApplyBGMVolume()
	{
		if (m_bgmInstance && !m_bgmFadingIn)
			m_bgmInstance->SetVolume(m_bgmVolume);
	}
}
