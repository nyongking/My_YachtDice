#include "GameEnginePch.h"
#include "CameraController.h"
#include "CameraComponent.h"
#include "InputManager.h"
#include "GameObject.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#endif

namespace GameEngine
{
	void CameraController::Start()
	{
		m_camera = GetOwner()->GetComponent<CameraComponent>();
	}

	void CameraController::EnsureInitialized()
	{
		if (m_initialized || !m_camera)
			return;

		using namespace DirectX;
		const float3& eye    = m_camera->GetEye();
		const float3& target = m_camera->GetTarget();

		XMVECTOR dir = XMVector3Normalize(
			XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye)));

		float3 d;
		XMStoreFloat3(&d, dir);

		m_yaw   = atan2f(d.x, d.z);
		m_pitch = asinf(-d.y);
		m_initialized = true;
	}

	void CameraController::Update(float dt)
	{
		if (!m_camera)
			return;

		EnsureInitialized();
		UpdateRotation(dt);
		UpdateMovement(dt);
	}

	void CameraController::UpdateMovement(float dt)
	{
		using namespace DirectX;
		auto& input = InputManager::GetInstance();

		const float3& eye    = m_camera->GetEye();
		const float3& target = m_camera->GetTarget();

		XMVECTOR forward = XMVector3Normalize(
			XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&eye)));
		XMVECTOR worldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		XMVECTOR right   = XMVector3Normalize(XMVector3Cross(worldUp, forward));

		XMVECTOR movement = XMVectorZero();

		if (input.IsKeyDown(Keyboard::Keys::W))
			movement = XMVectorAdd(movement, forward);
		if (input.IsKeyDown(Keyboard::Keys::S))
			movement = XMVectorSubtract(movement, forward);
		if (input.IsKeyDown(Keyboard::Keys::A))
			movement = XMVectorSubtract(movement, right);
		if (input.IsKeyDown(Keyboard::Keys::D))
			movement = XMVectorAdd(movement, right);
		if (input.IsKeyDown(Keyboard::Keys::E))
			movement = XMVectorAdd(movement, worldUp);
		if (input.IsKeyDown(Keyboard::Keys::Q))
			movement = XMVectorSubtract(movement, worldUp);

		if (XMVectorGetX(XMVector3LengthSq(movement)) > 0.0001f)
		{
			float speed = m_moveSpeed;
			if (input.IsKeyDown(Keyboard::Keys::LeftShift))
				speed *= 3.f;

			XMVECTOR delta = XMVectorScale(XMVector3Normalize(movement), speed * dt);

			float3 newEye, newTarget;
			XMStoreFloat3(&newEye,    XMVectorAdd(XMLoadFloat3(&eye),    delta));
			XMStoreFloat3(&newTarget, XMVectorAdd(XMLoadFloat3(&target), delta));

			m_camera->SetEye(newEye);
			m_camera->SetTarget(newTarget);
		}
	}

	void CameraController::UpdateRotation(float dt)
	{
		using namespace DirectX;
		auto& input = InputManager::GetInstance();

		if (!input.IsMouseButtonDown(1))
			return;

		float2 mouseDelta = input.GetMouseDelta();
		if (fabsf(mouseDelta.x) < 0.001f && fabsf(mouseDelta.y) < 0.001f)
			return;

		m_yaw   += mouseDelta.x * m_rotateSpeed * (XM_PI / 180.f);
		m_pitch += mouseDelta.y * m_rotateSpeed * (XM_PI / 180.f);

		constexpr float limit = XM_PIDIV2 - 0.01f;
		m_pitch = std::clamp(m_pitch, -limit, limit);

		float3 dir;
		dir.x = cosf(m_pitch) * sinf(m_yaw);
		dir.y = -sinf(m_pitch);
		dir.z = cosf(m_pitch) * cosf(m_yaw);

		const float3& eye = m_camera->GetEye();
		XMVECTOR dirV = XMVector3Normalize(XMLoadFloat3(&dir));

		float3 newTarget;
		XMStoreFloat3(&newTarget, XMVectorAdd(XMLoadFloat3(&eye), dirV));
		m_camera->SetTarget(newTarget);
	}

	MyJson CameraController::Serialize() const
	{
		MyJson j;
		j["type"]        = GetTypeName();
		j["moveSpeed"]   = m_moveSpeed;
		j["rotateSpeed"] = m_rotateSpeed;
		return j;
	}

	void CameraController::Deserialize(const MyJson& j)
	{
		m_moveSpeed   = j.value("moveSpeed",   10.f);
		m_rotateSpeed = j.value("rotateSpeed", 0.3f);
		m_initialized = false;
	}

#ifdef _DEBUG
	void CameraController::OnInspectorGUI()
	{
		if (ImGui::CollapsingHeader("Camera Controller", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat("Move Speed",   &m_moveSpeed,   0.1f, 0.1f, 100.f);
			ImGui::DragFloat("Rotate Speed", &m_rotateSpeed, 0.01f, 0.01f, 5.f);
		}
	}
#endif
}
