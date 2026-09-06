#pragma once
#include "ResourceCache.h"
#include "AudioClip.h"
#include "EngineGlobal.h"

namespace GameEngine
{
	class AudioManager : public ResourceCache<AudioClip>
	{
#pragma region Singleton
	public:
		static AudioManager* GetInstance() { return GAudioManager; }
		AudioManager() = default;
		AudioManager(const AudioManager&) = delete;
		AudioManager& operator=(const AudioManager&) = delete;
#pragma endregion Singleton

	public:
		bool Initialize();
		void Update(float dt);
		void Shutdown();

		// ---- Resource loading ----
		AudioClip* LoadSync(const std::string& key, const std::wstring& path);
		void LoadAsync(const std::string& key, const std::wstring& path,
		               std::function<void(AudioClip*)> onComplete = nullptr);
		AudioClip* Get(const std::string& key) const { return GetCached(key); }

		// ---- Volume control (3 channels) ----
		void  SetMasterVolume(float v);
		float GetMasterVolume() const { return m_masterVolume; }

		void  SetSFXVolume(float v)  { m_sfxVolume = std::clamp(v, 0.f, 1.f); }
		float GetSFXVolume()  const  { return m_sfxVolume; }

		void  SetBGMVolume(float v);
		float GetBGMVolume()  const  { return m_bgmVolume; }

		// ---- BGM playback (centrally managed) ----
		void PlayBGM(const std::string& clipKey, float fadeInSec = 0.f);
		void StopBGM(float fadeOutSec = 0.f);
		void PauseBGM();
		void ResumeBGM();
		bool IsBGMPlaying() const;

		// ---- SFX convenience (fire-and-forget, no GameObject) ----
		void PlaySFX(const std::string& clipKey,
		             float volume = 1.f, float pitch = 0.f, float pan = 0.f);

		// ---- 3D audio listener ----
		void SetActiveListener(const DirectX::AudioListener& listener);
		const DirectX::AudioListener& GetActiveListener() const { return m_listener; }
		bool HasActiveListener() const { return m_hasListener; }

		DirectX::AudioEngine* GetAudioEngine() { return m_audioEngine.get(); }

	private:
		void UpdateBGMFade(float dt);
		void ApplyBGMVolume();

		std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

		// Volume channels
		float m_masterVolume = 1.f;
		float m_sfxVolume    = 1.f;
		float m_bgmVolume    = 1.f;

		// BGM state
		std::unique_ptr<DirectX::SoundEffectInstance> m_bgmInstance;
		std::unique_ptr<DirectX::SoundEffectInstance> m_bgmFadeOutInstance;
		std::string m_currentBGMKey;

		float m_bgmFadeTimer      = 0.f;
		float m_bgmFadeDuration   = 0.f;
		float m_bgmFadeOutTimer   = 0.f;
		float m_bgmFadeOutDuration = 0.f;
		bool  m_bgmFadingIn       = false;
		bool  m_bgmFadingOut      = false;

		// 3D listener
		DirectX::AudioListener m_listener;
		bool m_hasListener = false;
	};
}
