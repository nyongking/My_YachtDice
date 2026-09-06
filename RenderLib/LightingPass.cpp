#include "RenderPch.h"
#include "LightingPass.h"

#include "RenderTargetGroup.h"
#include "ShaderGroup.h"
#include "Material.h"
#include "DeferredLightingMaterial.h"

namespace Render
{
	void LightingPass::SubmitLight(const LightCommand& cmd)
	{
		m_lights.push_back(cmd);
	}

	void LightingPass::Execute(ID3D11DeviceContext* ctx)
	{
		// 이전 프레임 G-Buffer SRV 해제 (RTV 충돌 방지)
		ID3D11ShaderResourceView* nullSRVs[3] = {};
		ctx->PSSetShaderResources(0, 3, nullSRVs);

		// 머티리얼 또는 G-Buffer가 준비되지 않은 경우 스킵
		if (!m_gBuffer || !m_material)
			return;

		// 파이프라인 스테이트 바인딩 (BlendState 등)
		BindPipelineState(ctx);

		// 카메라 위치 → 머티리얼에 전달
		if (auto* deferredMat = dynamic_cast<DeferredLightingMaterial*>(m_material))
			deferredMat->SetCameraPosition(m_cameraPos);

		// 제출된 Light 데이터 → 머티리얼에 전달
		if (!m_lights.empty())
		{
			std::vector<LightData> datas;
			datas.reserve(m_lights.size());
			for (const auto& cmd : m_lights)
				datas.push_back(cmd.data);
			m_material->SetLights(datas.data(), static_cast<int>(datas.size()));
		}
		else
		{
			m_material->SetLights(nullptr, 0);
		}

		// G-Buffer SRV → 머티리얼 텍스처 슬롯 바인딩
		m_material->SetTexture("gAlbedo",   m_gBuffer->GetSRV(0));
		m_material->SetTexture("gNormal",   m_gBuffer->GetSRV(1));
		m_material->SetTexture("gWorldPos", m_gBuffer->GetSRV(2));

		// 셰이더 + InputLayout 바인딩 (fullscreen VS는 InputLayout null)
		if (auto* sg = m_material->GetShaderGroup())
			sg->BindShaderAndLayout(ctx);

		ctx->IASetInputLayout(nullptr);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 머티리얼 CB 바인딩
		if (!m_material->BindMaterial(ctx))
			return;

		// 풀스크린 삼각형 (VB 없음, SV_VertexID 사용)
		ctx->Draw(3, 0);

		// G-Buffer SRV 해제 (다음 프레임 G-Buffer 쓰기 전 충돌 방지)
		ctx->PSSetShaderResources(0, 3, nullSRVs);

		// 파이프라인 스테이트 복원
		RestorePipelineState(ctx);
	}

	void LightingPass::Clear()
	{
		m_lights.clear();
	}
}
