#include "RenderPch.h"
#include "RenderPipeline.h"

#include "RenderDevice.h"
#include "Renderer.h"

namespace Render
{
	bool RenderPipeline::Initialize(UINT sizeX, UINT sizeY)
	{
		auto device = RenderDevice::GetInstance().GetDevice();

		// Geometry 패스 생성 (Opaque, Transparent, Effect)
		for (int i = 0; i < PASS_COUNT; ++i)
		{
			auto layer = static_cast<RenderPassBase::Layer>(i);

			if (layer == RenderPassBase::Layer::UI)
			{
				m_passes[i] = std::make_unique<UIPass>();
			}
			else
			{
				m_passes[i] = std::make_unique<GeometryPass>(layer);
			}

			if (!m_passes[i]->Initialize(device.Get()))
				return false;
		}

		// G-Buffer MRT 생성 (Albedo+Metallic / Normal+Roughness / WorldPos)

		m_gBuffer = std::make_unique<RenderTargetGroup>();
		if (!m_gBuffer->Create(device.Get(), sizeX, sizeY,
			{
				{ DXGI_FORMAT_R16G16B16A16_FLOAT },  // RT0: Albedo + Metallic
				{ DXGI_FORMAT_R16G16B16A16_FLOAT },  // RT1: Normal + Roughness
				{ DXGI_FORMAT_R32G32B32A32_FLOAT },  // RT2: World Position
			}, true))
			return false;

		// Opaque 패스 → G-Buffer 렌더 타겟 연결
		m_passes[static_cast<int>(RenderPassBase::Layer::Opaque)]->SetRenderTarget(m_gBuffer.get());

		// Lighting 패스 생성 및 G-Buffer 연결
		m_lightingPass = std::make_unique<LightingPass>();
		m_lightingPass->SetGBuffer(m_gBuffer.get());

		// 파이프라인 스테이트 생성
		if (!CreateBlendStates(device.Get()))
			return false;

		if (!CreateDepthStencilStates(device.Get()))
			return false;

		// 패스별 스테이트 연결
		m_passes[static_cast<int>(RenderPassBase::Layer::Transparent)]->SetBlendState(m_alphaBlendState.Get());
		m_passes[static_cast<int>(RenderPassBase::Layer::Effect)]->SetBlendState(m_additiveBlendState.Get());

		// UI 패스: AlphaBlend + 깊이 테스트/쓰기 Off
		m_passes[static_cast<int>(RenderPassBase::Layer::UI)]->SetBlendState(m_alphaBlendState.Get());
		m_passes[static_cast<int>(RenderPassBase::Layer::UI)]->SetDepthStencilState(m_depthDisabledState.Get());

		return true;
	}

	bool RenderPipeline::CreateBlendStates(ID3D11Device* device)
	{
		D3D11_BLEND_DESC desc = {};

		// Alpha Blend: SrcAlpha / InvSrcAlpha
		desc.RenderTarget[0].BlendEnable           = TRUE;
		desc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		if (FAILED(device->CreateBlendState(&desc, m_alphaBlendState.GetAddressOf())))
			return false;

		// Additive: SrcAlpha / One
		desc.RenderTarget[0].DestBlend      = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

		if (FAILED(device->CreateBlendState(&desc, m_additiveBlendState.GetAddressOf())))
			return false;

		return true;
	}

	bool RenderPipeline::CreateDepthStencilStates(ID3D11Device* device)
	{
		D3D11_DEPTH_STENCIL_DESC desc = {};

		// 깊이 테스트 Off, 깊이 쓰기 Off
		desc.DepthEnable    = FALSE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.StencilEnable  = FALSE;

		if (FAILED(device->CreateDepthStencilState(&desc, m_depthDisabledState.GetAddressOf())))
			return false;

		return true;
	}

	void RenderPipeline::Submit(RenderPassBase::Layer layer, const RenderCommand& cmd)
	{
		int idx = static_cast<int>(layer);
		if (idx < 0 || idx >= PASS_COUNT)
			return;

		m_passes[idx]->Submit(cmd);
	}

	void RenderPipeline::SubmitLight(const LightCommand& cmd)
	{
		m_lightingPass->SubmitLight(cmd);
	}

	void RenderPipeline::SubmitText(const TextCommand& cmd)
	{
		int idx = static_cast<int>(RenderPassBase::Layer::UI);
		auto* uiPass = static_cast<UIPass*>(m_passes[idx].get());
		if (uiPass)
			uiPass->SubmitText(cmd);
	}

	void RenderPipeline::BeginFrame()
	{
		// G-Buffer 클리어
		if (m_gBuffer)
		{
			auto ctx = RenderDevice::GetInstance().GetContext();
			m_gBuffer->Clear(ctx.Get(), float4(0.f, 0.f, 0.f, 0.f));
		}
	}

	void RenderPipeline::Execute(ID3D11DeviceContext* ctx)
	{
		// 1. GBuffer Pass — Opaque 오브젝트를 G-Buffer MRT에 기록
		m_passes[static_cast<int>(RenderPassBase::Layer::Opaque)]->Execute(ctx);

		// 2. Lighting Pass — G-Buffer SRV 읽기, 백버퍼에 조명 출력
		Renderer::GetInstance().BindBackbuffer(ctx);
		m_lightingPass->Execute(ctx);

		// 3. Transparent — Forward, 백버퍼 직접 출력
		m_passes[static_cast<int>(RenderPassBase::Layer::Transparent)]->Execute(ctx);

		// 4. Effect — G-Buffer depth를 백버퍼 depth로 복사하여 씬 기하와 깊이 테스트
		if (m_gBuffer && m_gBuffer->GetDepthTexture())
			Renderer::GetInstance().CopyDepthFrom(ctx, m_gBuffer->GetDepthTexture());
		m_passes[static_cast<int>(RenderPassBase::Layer::Effect)]->Execute(ctx);

		// 5. UI — 모든 것 위에 최종 렌더링 (깊이 테스트 Off)
		m_passes[static_cast<int>(RenderPassBase::Layer::UI)]->Execute(ctx);
	}

	void RenderPipeline::EndFrame()
	{
		for (auto& pass : m_passes)
			pass->Clear();

		m_lightingPass->Clear();
	}
}
