#pragma once
#include "GeometryPass.h"
#include "UIPass.h"
#include "LightingPass.h"
#include "RenderTargetGroup.h"
#include "TextItem.h"

namespace Render
{
	class RenderPipeline
	{
	public:
		static RenderPipeline& GetInstance()
		{
			static RenderPipeline instance;
			return instance;
		}

	public:
		bool Initialize(UINT sizeX, UINT sizeY);

		// 초기화 후 1회 설정
		void SetLightingMaterial(class Material* mat) { m_lightingPass->SetMaterial(mat); }
		void SetCameraPosition(const float3& pos)     { m_lightingPass->SetCameraPosition(pos); }

		// 게임 오브젝트가 매 프레임 호출
		void Submit(RenderPassBase::Layer layer, const RenderCommand& cmd);
		void SubmitLight(const LightCommand& cmd);
		void SubmitText(const TextCommand& cmd);

		// Renderer가 프레임마다 호출
		void BeginFrame();
		void Execute(ID3D11DeviceContext* ctx);
		void EndFrame();

	private:
		RenderPipeline() = default;
		~RenderPipeline() = default;
		RenderPipeline(const RenderPipeline&)            = delete;
		RenderPipeline& operator=(const RenderPipeline&) = delete;

		bool CreateBlendStates(ID3D11Device* device);
		bool CreateDepthStencilStates(ID3D11Device* device);

		// 렌더 패스 (Opaque/Transparent/Effect = GeometryPass, UI = UIPass)
		static constexpr int PASS_COUNT = static_cast<int>(RenderPassBase::Layer::COUNT);
		std::array<std::unique_ptr<RenderPassBase>, PASS_COUNT> m_passes;

		// Deferred 전용
		std::unique_ptr<RenderTargetGroup> m_gBuffer;
		std::unique_ptr<LightingPass>      m_lightingPass;

		// 공유 파이프라인 스테이트
		RefCom<ID3D11BlendState>        m_alphaBlendState;
		RefCom<ID3D11BlendState>        m_additiveBlendState;
		RefCom<ID3D11DepthStencilState> m_depthDisabledState;
	};
}
