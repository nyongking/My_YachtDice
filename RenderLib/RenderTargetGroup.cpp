#include "RenderPch.h"
#include "RenderTargetGroup.h"

namespace Render
{
	bool RenderTargetGroup::Create(ID3D11Device* device, uint32_t width, uint32_t height,
		const std::vector<RTDesc>& rts, bool hasDepth)
	{
		if (nullptr == device)
			return false;

		m_targets.clear();
		m_depthTexture.Reset();
		m_dsv.Reset();

		for (const auto& rt : rts)
		{
			Target target;

			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Width     = width;
			texDesc.Height    = height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format    = rt.format;
			texDesc.SampleDesc.Count   = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage     = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

			if (FAILED(device->CreateTexture2D(&texDesc, nullptr, target.texture.GetAddressOf())))
				return false;

			if (FAILED(device->CreateRenderTargetView(target.texture.Get(), nullptr, target.rtv.GetAddressOf())))
				return false;

			if (FAILED(device->CreateShaderResourceView(target.texture.Get(), nullptr, target.srv.GetAddressOf())))
				return false;

			m_targets.push_back(std::move(target));
		}

		if (hasDepth)
		{
			D3D11_TEXTURE2D_DESC depthDesc = {};
			depthDesc.Width     = width;
			depthDesc.Height    = height;
			depthDesc.MipLevels = 1;
			depthDesc.ArraySize = 1;
			depthDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthDesc.SampleDesc.Count   = 1;
			depthDesc.SampleDesc.Quality = 0;
			depthDesc.Usage     = D3D11_USAGE_DEFAULT;
			depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			if (FAILED(device->CreateTexture2D(&depthDesc, nullptr, m_depthTexture.GetAddressOf())))
				return false;

			if (FAILED(device->CreateDepthStencilView(m_depthTexture.Get(), nullptr, m_dsv.GetAddressOf())))
				return false;
		}

		return true;
	}

	void RenderTargetGroup::Bind(ID3D11DeviceContext* ctx)
	{
		std::vector<ID3D11RenderTargetView*> rtvs;
		rtvs.reserve(m_targets.size());

		for (auto& t : m_targets)
			rtvs.push_back(t.rtv.Get());

		ctx->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), m_dsv.Get());
	}

	void RenderTargetGroup::Clear(ID3D11DeviceContext* ctx, const float4& color)
	{
		for (auto& t : m_targets)
			ctx->ClearRenderTargetView(t.rtv.Get(), &color.x);

		if (m_dsv)
			ctx->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}

	ID3D11ShaderResourceView* RenderTargetGroup::GetSRV(uint32_t index)
	{
		if (index >= static_cast<uint32_t>(m_targets.size()))
			return nullptr;

		return m_targets[index].srv.Get();
	}
}
