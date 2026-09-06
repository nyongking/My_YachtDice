#include "GameEnginePch.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "RenderPipeline.h"
#include "Renderer.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#endif

namespace GameEngine
{
	void CameraComponent::SetPerspective(float fovY, float aspect, float nearZ, float farZ)
	{
		using namespace DirectX;

		m_isPerspective = true;
		m_fovYDeg       = fovY * (180.f / XM_PI);
		m_aspect        = aspect;
		m_nearZ         = nearZ;
		m_farZ          = farZ;

		XMMATRIX proj = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
		m_matrix.UpdateProj(proj);
	}

	void CameraComponent::SetOrthographic(float width, float height, float nearZ, float farZ)
	{
		using namespace DirectX;

		m_isPerspective = false;
		m_orthoWidth    = width;
		m_orthoHeight   = height;
		m_nearZ         = nearZ;
		m_farZ          = farZ;

		XMMATRIX proj = XMMatrixOrthographicLH(width, height, nearZ, farZ);
		m_matrix.UpdateProj(proj);
	}

	void CameraComponent::SetLookAt(DirectX::FXMVECTOR eye,
	                                 DirectX::FXMVECTOR target,
	                                 DirectX::FXMVECTOR up)
	{
		using namespace DirectX;
		XMMATRIX view = XMMatrixLookAtLH(eye, target, up);
		m_matrix.UpdateView(view);
	}

	MyJson CameraComponent::Serialize() const
	{
		MyJson j;
		j["type"]          = GetTypeName();
		j["isMainCamera"]  = m_isMainCamera;
		j["isPerspective"] = m_isPerspective;
		j["nearZ"]         = m_nearZ;
		j["farZ"]          = m_farZ;
		j["eye"]           = { m_eye.x,    m_eye.y,    m_eye.z    };
		j["target"]        = { m_target.x, m_target.y, m_target.z };

		if (m_isPerspective)
		{
			j["fovYDeg"] = m_fovYDeg;
			j["aspect"]  = m_aspect;
		}
		else
		{
			j["orthoWidth"]  = m_orthoWidth;
			j["orthoHeight"] = m_orthoHeight;
		}

		return j;
	}

	void CameraComponent::Deserialize(const MyJson& j)
	{
		using namespace DirectX;

		bool  isPerspective = j.value("isPerspective", true);
		float nearZ         = j.value("nearZ",  0.1f);
		float farZ          = j.value("farZ",  1000.f);

		if (isPerspective)
		{
			float fovYDeg = j.value("fovYDeg", 60.f);
			float aspect  = j.value("aspect",  16.f / 9.f);
			SetPerspective(fovYDeg * (XM_PI / 180.f), aspect, nearZ, farZ);
		}
		else
		{
			float w = j.value("orthoWidth",  10.f);
			float h = j.value("orthoHeight", 10.f);
			SetOrthographic(w, h, nearZ, farZ);
		}

		if (j.contains("eye"))
			m_eye    = { j["eye"][0],    j["eye"][1],    j["eye"][2]    };
		if (j.contains("target"))
			m_target = { j["target"][0], j["target"][1], j["target"][2] };

		// View matrix 초기화 (SetLookAt 없이 방치 시 행렬이 0으로 남음)
		SetLookAt(
			XMVectorSet(m_eye.x,    m_eye.y,    m_eye.z,    1.f),
			XMVectorSet(m_target.x, m_target.y, m_target.z, 1.f),
			XMVectorSet(0.f, 1.f, 0.f, 0.f));

		m_isMainCamera = j.value("isMainCamera", false);
		if (m_isMainCamera)
			CameraManager::GetInstance().SetMainCamera(this);
	}

#ifdef _DEBUG
	void CameraComponent::OnInspectorGUI()
	{
		// Eye / Target
		if (ImGui::CollapsingHeader("View", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool viewChanged = false;
			viewChanged |= ImGui::DragFloat3("Eye",    &m_eye.x,    0.01f);
			viewChanged |= ImGui::DragFloat3("Target", &m_target.x, 0.01f);
			if (viewChanged)
				SetLookAt(
					DirectX::XMVectorSet(m_eye.x,    m_eye.y,    m_eye.z,    1.f),
					DirectX::XMVectorSet(m_target.x, m_target.y, m_target.z, 1.f),
					DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f));
		}

		// Projection
		bool changed = false;
		if (ImGui::Checkbox("Perspective", &m_isPerspective))
			changed = true;

		if (m_isPerspective)
		{
			changed |= ImGui::DragFloat("FOV (deg)", &m_fovYDeg, 0.5f, 1.f, 179.f);
			changed |= ImGui::DragFloat("Aspect",    &m_aspect,  0.01f, 0.1f, 10.f);
			changed |= ImGui::DragFloat("Near",      &m_nearZ,   0.001f, 0.001f, 100.f);
			changed |= ImGui::DragFloat("Far",       &m_farZ,    1.f, 1.f, 100000.f);
			if (changed)
				SetPerspective(m_fovYDeg * (DirectX::XM_PI / 180.f), m_aspect, m_nearZ, m_farZ);
		}
		else
		{
			changed |= ImGui::DragFloat("Width",  &m_orthoWidth,  1.f, 0.1f, 10000.f);
			changed |= ImGui::DragFloat("Height", &m_orthoHeight, 1.f, 0.1f, 10000.f);
			changed |= ImGui::DragFloat("Near",   &m_nearZ,       0.001f, 0.001f, 100.f);
			changed |= ImGui::DragFloat("Far",    &m_farZ,        1.f, 1.f, 100000.f);
			if (changed)
				SetOrthographic(m_orthoWidth, m_orthoHeight, m_nearZ, m_farZ);
		}
	}
#endif

	Ray CameraComponent::ScreenToWorldRay(float screenX, float screenY) const
	{
		using namespace DirectX;

		// 뷰포트 크기
		float w = static_cast<float>(Render::Renderer::GetInstance().GetWidth());
		float h = static_cast<float>(Render::Renderer::GetInstance().GetHeight());

		// 스크린 → NDC (-1 ~ +1)
		float ndcX = (2.f * screenX / w) - 1.f;
		float ndcY = 1.f - (2.f * screenY / h);  // Y 반전

		// NDC → 뷰 공간 (near plane)
		XMMATRIX proj = XMLoadFloat4x4(GetProj());
		XMMATRIX invProj = XMMatrixInverse(nullptr, proj);

		XMVECTOR nearPoint = XMVector3TransformCoord(
			XMVectorSet(ndcX, ndcY, 0.f, 1.f), invProj);
		XMVECTOR farPoint = XMVector3TransformCoord(
			XMVectorSet(ndcX, ndcY, 1.f, 1.f), invProj);

		// 뷰 공간 → 월드 공간
		XMMATRIX view = XMLoadFloat4x4(GetView());
		XMMATRIX invView = XMMatrixInverse(nullptr, view);

		XMVECTOR worldNear = XMVector3TransformCoord(nearPoint, invView);
		XMVECTOR worldFar  = XMVector3TransformCoord(farPoint, invView);

		XMVECTOR dir = XMVector3Normalize(worldFar - worldNear);

		XMFLOAT3 o, d;
		XMStoreFloat3(&o, worldNear);
		XMStoreFloat3(&d, dir);

		Ray ray;
		ray.origin    = Vec3(o.x, o.y, o.z);
		ray.direction = Vec3(d.x, d.y, d.z);

		return ray;
	}

	void CameraComponent::LateUpdate(float dt)
	{
		using namespace DirectX;

		SetLookAt(
			XMVectorSet(m_eye.x,    m_eye.y,    m_eye.z,    1.f),
			XMVectorSet(m_target.x, m_target.y, m_target.z, 1.f),
			XMVectorSet(0.f, 1.f, 0.f, 0.f));

		// 메인 카메라일 때 카메라 위치를 RenderPipeline에 전달 (Specular 계산용)
		if (CameraManager::GetInstance().GetMainCamera() == this)
		{
			const float4x4* invView = GetInverseView();
			if (invView)
				Render::RenderPipeline::GetInstance().SetCameraPosition({ invView->_41, invView->_42, invView->_43 });
		}
	}
}
