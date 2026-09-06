#pragma once
#include "Component.h"
#include "ViewProjMatrix.h"
#include "Ray.h"

namespace GameEngine
{
	class CameraComponent : public Component
	{
	public:
		void SetPerspective(float fovY, float aspect, float nearZ, float farZ);
		void SetOrthographic(float width, float height, float nearZ, float farZ);

		void SetLookAt(DirectX::FXMVECTOR eye,
		               DirectX::FXMVECTOR target,
		               DirectX::FXMVECTOR up);

		void SetEye(const float3& eye)       { m_eye = eye; }
		void SetTarget(const float3& target) { m_target = target; }
		const float3& GetEye()    const { return m_eye; }
		const float3& GetTarget() const { return m_target; }

		void LateUpdate(float dt) override;

		const float4x4* GetViewProj()    const { return m_matrix.GetViewProj(); }
		const float4x4* GetView()        const { return m_matrix.GetView(); }
		const float4x4* GetProj()        const { return m_matrix.GetProj(); }
		const float4x4* GetInverseView() const { return m_matrix.GetInverseView(); }

		// 스크린 좌표(px) → 월드 레이 변환
		Ray ScreenToWorldRay(float screenX, float screenY) const;

		// 직렬화
		std::string GetTypeName()            const override { return "CameraComponent"; }
		MyJson      Serialize()              const override;
		void        Deserialize(const MyJson& j)   override;

#ifdef _DEBUG
		void OnInspectorGUI() override;
#endif

	protected:
		Render::ViewProjMatrix m_matrix;

		float3 m_eye    = { 0.f, 5.f, -10.f };
		float3 m_target = { 0.f, 0.f,   0.f };

		// 직렬화를 위한 투영 파라미터 저장
		bool  m_isMainCamera  = false;
		bool  m_isPerspective = true;
		float m_fovYDeg       = 60.f;
		float m_aspect        = 16.f / 9.f;
		float m_nearZ         = 0.1f;
		float m_farZ          = 1000.f;
		float m_orthoWidth    = 10.f;
		float m_orthoHeight   = 10.f;
	};
}
