#pragma once
#include "Component.h"
#include "DirectXTK/Audio.h"

namespace GameEngine
{
	class AudioListenerComponent : public Component
	{
	public:
		AudioListenerComponent()  = default;
		~AudioListenerComponent() = default;

	public:
		void Start()              override;
		void LateUpdate(float dt) override;
		void OnDestroy()          override;

		// Serialization
		std::string GetTypeName()            const override { return "AudioListenerComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		DirectX::AudioListener m_listener;
		float3 m_lastPosition = { 0.f, 0.f, 0.f };
	};
}
