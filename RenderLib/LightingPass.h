#pragma once
#include "RenderPassBase.h"
#include "LightItem.h"

namespace Render
{
	class Material;
	class RenderTargetGroup;

	class LightingPass : public RenderPassBase
	{
	public:
		LightingPass()  = default;
		~LightingPass() = default;

	public:
		bool Initialize(ID3D11Device*) override { return true; }
		void Submit(const RenderCommand&) override {}  // LightingPass는 SubmitLight 사용

		void SubmitLight(const LightCommand& cmd);

		void SetGBuffer(RenderTargetGroup* gbuffer) { m_gBuffer = gbuffer; }
		void SetMaterial(Material* mat)             { m_material = mat; }
		void SetCameraPosition(const float3& pos)   { m_cameraPos = pos; }

		void Execute(ID3D11DeviceContext* ctx) override;
		void Clear() override;

	private:
		RenderTargetGroup*        m_gBuffer  = nullptr;
		Material*                 m_material = nullptr;
		float3                    m_cameraPos = {};

		std::vector<LightCommand> m_lights;
	};
}
