#pragma once
#include "Component.h"
#include "DirectXTK/Audio.h"
#include <string>

namespace GameEngine
{
	class AudioClip;

	class AudioSourceComponent : public Component
	{
	public:
		AudioSourceComponent()  = default;
		~AudioSourceComponent() = default;

	public:
		void Awake()            override;
		void Update(float dt)   override;
		void OnDestroy()        override;

		// Configuration
		void SetClip(const std::string& key, const std::wstring& path);
		void SetLoop(bool loop)          { m_loop = loop; }
		void SetVolume(float v)          { m_volume = v; }
		void SetPitch(float p)           { m_pitch = p; }
		void SetPlayOnAwake(bool v)      { m_playOnAwake = v; }
		void SetIs3D(bool v)             { m_is3D = v; }
		void SetMinDistance(float d)     { m_emitter.InnerRadius = d; }
		void SetMaxDistance(float d)     { m_emitter.CurveDistanceScaler = d; }

		// Playback
		void Play();
		void Stop();
		void Pause();
		void Resume();
		bool IsPlaying() const;

		// Serialization
		std::string GetTypeName()            const override { return "AudioSourceComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		void Apply3D();

		std::string m_clipKey;
		std::string m_clipPath;
		AudioClip*  m_clip = nullptr;

		std::unique_ptr<DirectX::SoundEffectInstance> m_instance;
		DirectX::AudioEmitter m_emitter;

		float m_volume      = 1.f;
		float m_pitch       = 0.f;
		bool  m_loop        = false;
		bool  m_playOnAwake = false;
		bool  m_is3D        = false;
	};
}
