#pragma once

namespace Render
{
	class Renderer
	{
#pragma region Singleton
	public:
		static Renderer& GetInstance()
		{
			static Renderer instance;

			return instance;
		}
#pragma endregion Singleton

	public:
		Renderer();
		~Renderer();

	public:
		bool Initialize(bool window, UINT sizeX, UINT sizeY, HWND hwnd);
		bool CreateSamplers();

		void RenderBegin(DirectX::XMFLOAT4 clearColor = DirectX::XMFLOAT4(0.f, 0.f, 1.f, 1.f));
		void RenderEnd();

		// G-Buffer Pass 이후 백버퍼 RTV 복귀 (클리어 없음)
		void BindBackbuffer(ID3D11DeviceContext* ctx);

		// Depth buffer만 클리어 (Effect 패스 전 호출)
		void ClearDepth(ID3D11DeviceContext* ctx);

		// 외부 depth texture를 백버퍼 depth로 복사
		void CopyDepthFrom(ID3D11DeviceContext* ctx, ID3D11Texture2D* srcDepth);

		// 뷰포트 크기
		UINT GetWidth()  const { return m_width; }
		UINT GetHeight() const { return m_height; }

	private:
		UINT m_width  = 0;
		UINT m_height = 0;
		RefCom<ID3D11Device> m_device = nullptr;
		RefCom<ID3D11DeviceContext> m_context = nullptr;

		RefCom<IDXGISwapChain>			m_swapchain;
		RefCom<ID3D11RenderTargetView>	m_backbufferRTV;
		RefCom<ID3D11Texture2D>			m_depthTexture;
		RefCom<ID3D11DepthStencilView>	m_depthstencilView;

		std::vector<RefCom<ID3D11SamplerState>> m_samplers;
	};
}
