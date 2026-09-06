#pragma once
#include "Component.h"

namespace GameEngine
{
	class CameraComponent;

	class CameraController : public Component
	{
	public:
		void Start() override;
		void Update(float dt) override;

		void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
		void SetRotateSpeed(float speed) { m_rotateSpeed = speed; }

		// 직렬화
		std::string GetTypeName()            const override { return "CameraController"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	private:
		void UpdateMovement(float dt);
		void UpdateRotation(float dt);
		void EnsureInitialized();

		CameraComponent* m_camera = nullptr;

		float m_moveSpeed   = 10.f;
		float m_rotateSpeed = 0.3f;   // 도/픽셀

		float m_yaw   = 0.f;
		float m_pitch = 0.f;
		bool  m_initialized = false;
	};
}
