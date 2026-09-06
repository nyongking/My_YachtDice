#include "RenderPch.h"
#include "GeometryPass.h"

#include "RenderTargetGroup.h"
#include "ShaderGroup.h"
#include "Material.h"
#include "Geometry.h"

#include <algorithm>

namespace Render
{
	bool GeometryPass::Initialize(ID3D11Device* device)
	{
		if (!device)
			return false;

		if (!m_cbPerFrame.Create(device, sizeof(float4x4), nullptr, D3D11_USAGE_DYNAMIC))
			return false;

		if (!m_cbPerObject.Create(device, sizeof(float4x4), nullptr, D3D11_USAGE_DYNAMIC))
			return false;

		return true;
	}

	void GeometryPass::Submit(const RenderCommand& cmd)
	{
		m_queue.push_back(cmd);
	}

	void GeometryPass::Execute(ID3D11DeviceContext* ctx)
	{
		if (m_queue.empty())
			return;

		// RenderTarget 바인드 (nullptr이면 backbuffer 그대로 사용)
		if (m_renderTarget)
			m_renderTarget->Bind(ctx);

		// 파이프라인 스테이트 바인딩 (BlendState 등)
		BindPipelineState(ctx);

		Sort();

		// PerFrame CB (b0): viewProj — 패스 내 모든 오브젝트 동일, 1회 업로드
		m_cbPerFrame.Update(ctx, &m_queue[0].viewProj, sizeof(float4x4));
		m_cbPerFrame.BindVS(ctx, 0);

		ShaderGroup* currentShader = nullptr;

		for (const auto& cmd : m_queue)
		{
			if (nullptr == cmd.geometry || nullptr == cmd.material)
				continue;

			// PerObject CB (b1): world — 드로우콜마다 업로드
			m_cbPerObject.Update(ctx, &cmd.world, sizeof(float4x4));
			m_cbPerObject.BindVS(ctx, 1);

			ShaderGroup* sg = cmd.material->GetShaderGroup();

			// InputLayout + Shader 변경 시에만 바인드
			if (sg != currentShader)
			{
				if (sg)
					sg->BindShaderAndLayout(ctx);
				currentShader = sg;
			}

			// Material CBs 바인드
			if (false == cmd.material->BindMaterial(ctx))
				continue;

			// Geometry VB/IB 바인드 + DrawIndexed
			cmd.geometry->BindAndDraw(ctx);
		}

		// 파이프라인 스테이트 복원
		RestorePipelineState(ctx);
	}

	void GeometryPass::Clear()
	{
		m_queue.clear();
	}

	void GeometryPass::Sort()
	{
		// ShaderGroup 포인터 기준 정렬 → 동일 셰이더 드로우 묶기
		std::stable_sort(m_queue.begin(), m_queue.end(),
			[](const RenderCommand& a, const RenderCommand& b)
			{
				ShaderGroup* sgA = (a.material) ? a.material->GetShaderGroup() : nullptr;
				ShaderGroup* sgB = (b.material) ? b.material->GetShaderGroup() : nullptr;
				return sgA < sgB;
			});
	}
}
