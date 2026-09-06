#include "RenderPch.h"
#include "RenderPassBase.h"

namespace Render
{
	void RenderPassBase::BindPipelineState(ID3D11DeviceContext* ctx)
	{
		// BlendState
		if (m_blendState)
		{
			ctx->OMGetBlendState(&m_prevBlendState, m_prevBlendFactor, &m_prevSampleMask);
			FLOAT factor[4] = { 0.f, 0.f, 0.f, 0.f };
			ctx->OMSetBlendState(m_blendState, factor, 0xFFFFFFFF);
		}

		// DepthStencilState
		if (m_depthStencilState)
		{
			ctx->OMGetDepthStencilState(&m_prevDepthStencilState, &m_prevStencilRef);
			ctx->OMSetDepthStencilState(m_depthStencilState, 0);
		}
	}

	void RenderPassBase::RestorePipelineState(ID3D11DeviceContext* ctx)
	{
		// BlendState 복원
		if (m_blendState)
		{
			ctx->OMSetBlendState(m_prevBlendState, m_prevBlendFactor, m_prevSampleMask);
			if (m_prevBlendState)
			{
				m_prevBlendState->Release();
				m_prevBlendState = nullptr;
			}
		}

		// DepthStencilState 복원
		if (m_depthStencilState)
		{
			ctx->OMSetDepthStencilState(m_prevDepthStencilState, m_prevStencilRef);
			if (m_prevDepthStencilState)
			{
				m_prevDepthStencilState->Release();
				m_prevDepthStencilState = nullptr;
			}
		}
	}
}
