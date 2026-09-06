#pragma once

namespace Render
{
	class RenderTargetGroup;
	struct RenderCommand;

	class RenderPassBase
	{
	public:
		enum class Layer
		{
			Opaque      = 0,
			Transparent = 1,
			Effect      = 2,
			UI          = 3,
			COUNT
		};

		virtual ~RenderPassBase() = default;

		virtual bool Initialize(ID3D11Device* device) = 0;
		virtual void Submit(const RenderCommand& cmd) = 0;
		virtual void Execute(ID3D11DeviceContext* ctx) = 0;
		virtual void Clear() {}

		void SetRenderTarget(RenderTargetGroup* rtg)       { m_renderTarget = rtg; }
		void SetBlendState(ID3D11BlendState* bs)           { m_blendState = bs; }
		void SetDepthStencilState(ID3D11DepthStencilState* dss) { m_depthStencilState = dss; }

	protected:
		// Execute() 시작/끝에서 호출하여 파이프라인 스테이트 바인딩/복원
		void BindPipelineState(ID3D11DeviceContext* ctx);
		void RestorePipelineState(ID3D11DeviceContext* ctx);

		RenderTargetGroup*        m_renderTarget      = nullptr;
		ID3D11BlendState*         m_blendState        = nullptr;
		ID3D11DepthStencilState*  m_depthStencilState = nullptr;

	private:
		// 복원용 백업
		ID3D11BlendState*         m_prevBlendState        = nullptr;
		FLOAT                     m_prevBlendFactor[4]    = {};
		UINT                      m_prevSampleMask        = 0xFFFFFFFF;
		ID3D11DepthStencilState*  m_prevDepthStencilState = nullptr;
		UINT                      m_prevStencilRef        = 0;
	};
}
