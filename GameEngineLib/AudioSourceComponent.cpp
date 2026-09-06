#include "GameEnginePch.h"
#include "AudioSourceComponent.h"
#include "AudioManager.h"
#include "GameObject.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#endif

namespace GameEngine
{
	void AudioSourceComponent::Awake()
	{
		if (!m_clipKey.empty() && !m_clip)
		{
			std::wstring wpath(m_clipPath.begin(), m_clipPath.end());
			m_clip = GAudioManager->LoadSync(m_clipKey, wpath);
		}

		if (m_playOnAwake && m_clip)
			Play();
	}

	void AudioSourceComponent::Update(float dt)
	{
		if (!m_is3D || !m_instance)
			return;

		if (m_instance->GetState() == DirectX::PLAYING)
			Apply3D();
	}

	void AudioSourceComponent::OnDestroy()
	{
		if (m_instance)
		{
			m_instance->Stop();
			m_instance.reset();
		}
	}

	void AudioSourceComponent::SetClip(const std::string& key, const std::wstring& path)
	{
		m_clipKey = key;
		// Store narrow path for serialization
		m_clipPath.assign(path.begin(), path.end());
		m_clip = GAudioManager->LoadSync(key, path);
	}

	void AudioSourceComponent::Play()
	{
		if (!m_clip)
			return;

		DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags = DirectX::SoundEffectInstance_Default;
		if (m_is3D)
			flags |= DirectX::SoundEffectInstance_Use3D;

		m_instance = m_clip->CreateInstance(flags);
		if (!m_instance)
			return;

		float effectiveVolume = m_volume * GAudioManager->GetSFXVolume();
		m_instance->SetVolume(effectiveVolume);
		m_instance->SetPitch(m_pitch);

		if (m_is3D)
			Apply3D();

		m_instance->Play(m_loop);
	}

	void AudioSourceComponent::Stop()
	{
		if (m_instance)
		{
			m_instance->Stop();
			m_instance.reset();
		}
	}

	void AudioSourceComponent::Pause()
	{
		if (m_instance && m_instance->GetState() == DirectX::PLAYING)
			m_instance->Pause();
	}

	void AudioSourceComponent::Resume()
	{
		if (m_instance && m_instance->GetState() == DirectX::PAUSED)
			m_instance->Resume();
	}

	bool AudioSourceComponent::IsPlaying() const
	{
		return m_instance && m_instance->GetState() == DirectX::PLAYING;
	}

	void AudioSourceComponent::Apply3D()
	{
		if (!m_instance || !GAudioManager->HasActiveListener())
			return;

		auto* transform = GetOwner()->GetTransform();
		const float3& pos = transform->GetPosition();
		m_emitter.SetPosition(DirectX::XMFLOAT3(pos.x, pos.y, pos.z));

		m_instance->Apply3D(
			GAudioManager->GetActiveListener(),
			m_emitter,
			false); // LH coordinate system (DirectX)
	}

	// ---- Serialization ----

	MyJson AudioSourceComponent::Serialize() const
	{
		MyJson j;
		j["type"]        = GetTypeName();
		j["clipKey"]     = m_clipKey;
		j["clipPath"]    = m_clipPath;
		j["volume"]      = m_volume;
		j["pitch"]       = m_pitch;
		j["loop"]        = m_loop;
		j["playOnAwake"] = m_playOnAwake;
		j["is3D"]        = m_is3D;
		j["minDistance"]  = m_emitter.InnerRadius;
		j["maxDistance"]  = m_emitter.CurveDistanceScaler;
		return j;
	}

	void AudioSourceComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("clipKey"))     m_clipKey     = j["clipKey"].get<std::string>();
		if (j.contains("clipPath"))    m_clipPath    = j["clipPath"].get<std::string>();
		if (j.contains("volume"))      m_volume      = j["volume"];
		if (j.contains("pitch"))       m_pitch       = j["pitch"];
		if (j.contains("loop"))        m_loop        = j["loop"];
		if (j.contains("playOnAwake")) m_playOnAwake = j["playOnAwake"];
		if (j.contains("is3D"))        m_is3D        = j["is3D"];
		if (j.contains("minDistance")) m_emitter.InnerRadius        = j["minDistance"];
		if (j.contains("maxDistance")) m_emitter.CurveDistanceScaler = j["maxDistance"];
	}

	// ---- Inspector ----

#ifdef _DEBUG
	void AudioSourceComponent::OnInspectorGUI()
	{
		ImGui::Text("Clip: %s", m_clipKey.empty() ? "(none)" : m_clipKey.c_str());
		ImGui::Text("Path: %s", m_clipPath.empty() ? "(none)" : m_clipPath.c_str());

		ImGui::SliderFloat("Volume", &m_volume, 0.f, 1.f);
		ImGui::SliderFloat("Pitch",  &m_pitch,  -1.f, 1.f);
		ImGui::Checkbox("Loop",        &m_loop);
		ImGui::Checkbox("Play On Awake", &m_playOnAwake);
		ImGui::Checkbox("3D Audio",    &m_is3D);

		if (m_is3D)
		{
			float minDist = m_emitter.InnerRadius;
			float maxDist = m_emitter.CurveDistanceScaler;
			if (ImGui::DragFloat("Min Distance", &minDist, 0.1f, 0.f, 1000.f))
				m_emitter.InnerRadius = minDist;
			if (ImGui::DragFloat("Max Distance", &maxDist, 0.1f, 0.f, 10000.f))
				m_emitter.CurveDistanceScaler = maxDist;
		}

		bool playing = IsPlaying();
		if (playing)
		{
			if (ImGui::Button("Stop"))
				Stop();
		}
		else
		{
			if (ImGui::Button("Play"))
				Play();
		}
	}
#endif
}
