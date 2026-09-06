#pragma once

namespace Render
{
	class RenderTargetGroup
	{
	public:
		struct RTDesc { DXGI_FORMAT format; };

		bool Create(ID3D11Device* device, uint32_t width, uint32_t height,
			const std::vector<RTDesc>& rts, bool hasDepth = true);

		void Bind(ID3D11DeviceContext* ctx);
		void Clear(ID3D11DeviceContext* ctx, const float4& color);
		ID3D11ShaderResourceView*  GetSRV(uint32_t index);
		ID3D11DepthStencilView*    GetDSV() { return m_dsv.Get(); }
		ID3D11Texture2D*           GetDepthTexture() { return m_depthTexture.Get(); }

	private:
		struct Target
		{
			RefCom<ID3D11Texture2D>          texture;
			RefCom<ID3D11RenderTargetView>   rtv;
			RefCom<ID3D11ShaderResourceView> srv;
		};

		std::vector<Target>            m_targets;
		RefCom<ID3D11Texture2D>        m_depthTexture;
		RefCom<ID3D11DepthStencilView> m_dsv;
	};
}
