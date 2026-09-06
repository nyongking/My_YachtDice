#include "RenderPch.h"
#include "Renderer.h"

#include "RenderDevice.h"

namespace Render
{

	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

	bool Renderer::Initialize(bool window, UINT sizeX, UINT sizeY, HWND hwnd)
	{
		m_swapchain.Reset();
		m_backbufferRTV.Reset();
		m_depthTexture.Reset();
		m_depthstencilView.Reset();

		m_device = RenderDevice::GetInstance().m_device;
		m_context = RenderDevice::GetInstance().m_context;

#pragma region Swapchain
		IDXGIDevice* pDeviceTemp = nullptr;
		m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDeviceTemp);

		IDXGIAdapter* pAdapter = nullptr;
		pDeviceTemp->GetParent(__uuidof(IDXGIAdapter), (void**)&pAdapter);

		IDXGIFactory* pFactory = nullptr;
		pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);

		DXGI_SWAP_CHAIN_DESC		SwapChain;
		ZeroMemory(&SwapChain, sizeof(DXGI_SWAP_CHAIN_DESC));

		SwapChain.BufferDesc.Width = sizeX;
		SwapChain.BufferDesc.Height = sizeY;

		SwapChain.BufferDesc.RefreshRate.Numerator = 144;
		SwapChain.BufferDesc.RefreshRate.Denominator = 1;

		SwapChain.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		SwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		SwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		SwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChain.BufferCount = 1;

		SwapChain.SampleDesc.Quality = 0;
		SwapChain.SampleDesc.Count = 1;

		SwapChain.OutputWindow = hwnd;
		SwapChain.Windowed = window;
		SwapChain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		SwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		if (FAILED(pFactory->CreateSwapChain(m_device.Get(), &SwapChain, m_swapchain.GetAddressOf())))
			return false;

		pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

		pFactory->Release();
		pAdapter->Release();
		pDeviceTemp->Release();
#pragma endregion

#pragma region RTV

		ID3D11Texture2D* pBackBufferTexture = nullptr;

		if (FAILED(m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture)))
			return false;

		if (FAILED(m_device->CreateRenderTargetView(pBackBufferTexture, nullptr, m_backbufferRTV.GetAddressOf())))
			return false;

		pBackBufferTexture->Release();

#pragma endregion

#pragma region DSV
		D3D11_TEXTURE2D_DESC	TextureDesc;
		ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

		TextureDesc.Width = sizeX;
		TextureDesc.Height = sizeY;
		TextureDesc.MipLevels = 1;
		TextureDesc.ArraySize = 1;
		TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.SampleDesc.Count = 1;

		TextureDesc.Usage = D3D11_USAGE_DEFAULT;
		TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		TextureDesc.CPUAccessFlags = 0;
		TextureDesc.MiscFlags = 0;

		if (FAILED(m_device->CreateTexture2D(&TextureDesc, nullptr, m_depthTexture.GetAddressOf())))
			return false;

		if (FAILED(m_device->CreateDepthStencilView(m_depthTexture.Get(), nullptr, m_depthstencilView.GetAddressOf())))
			return false;
#pragma endregion

#pragma region Viewport
		ID3D11RenderTargetView* pRTVs[1] = {
			m_backbufferRTV.Get(),
		};

		m_context->OMSetRenderTargets(1, pRTVs,
			m_depthstencilView.Get());

		D3D11_VIEWPORT			ViewPortDesc;
		ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
		ViewPortDesc.TopLeftX = 0;
		ViewPortDesc.TopLeftY = 0;
		ViewPortDesc.Width = static_cast<FLOAT>(sizeX);
		ViewPortDesc.Height = static_cast<FLOAT>(sizeY);

		m_width  = sizeX;
		m_height = sizeY;
		ViewPortDesc.MinDepth = 0.f;
		ViewPortDesc.MaxDepth = 1.f;

		m_context->RSSetViewports(1, &ViewPortDesc);
#pragma endregion

#pragma region DefaultSamplers
		if (false == CreateSamplers())

			return false;

#pragma endregion	

		return true;
	}

	bool Renderer::CreateSamplers()
	{
		D3D11_SAMPLER_DESC samplerDesc = {};

		{
			memset(&samplerDesc, 0, sizeof(samplerDesc));

			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 1;

			RefCom<ID3D11SamplerState> samplerState;

			if (FAILED(m_device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf())))
				return false;

			m_samplers.push_back(samplerState);

			// ���� ����
			m_context->PSSetSamplers(0, 1, m_samplers[0].GetAddressOf());
		}

		{
			memset(&samplerDesc, 0, sizeof(samplerDesc));

			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 1;

			RefCom<ID3D11SamplerState> samplerState;

			if (FAILED(m_device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf())))
				return false;

			m_samplers.push_back(samplerState);

			// ���� ����
			m_context->PSSetSamplers(1, 1, m_samplers[1].GetAddressOf());
		}

		return true;
	}

	void Renderer::RenderBegin(DirectX::XMFLOAT4 clearColor)
	{
		ID3D11RenderTargetView* pRTVs[] = { m_backbufferRTV.Get() };
		m_context->OMSetRenderTargets(1, pRTVs, m_depthstencilView.Get());

		m_context->ClearRenderTargetView(m_backbufferRTV.Get(), &clearColor.x);
		m_context->ClearDepthStencilView(m_depthstencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}

	void Renderer::RenderEnd()
	{
		m_swapchain->Present(0, 0);
	}

	void Renderer::BindBackbuffer(ID3D11DeviceContext* ctx)
	{
		ID3D11RenderTargetView* rtvs[] = { m_backbufferRTV.Get() };
		ctx->OMSetRenderTargets(1, rtvs, m_depthstencilView.Get());
	}

	void Renderer::ClearDepth(ID3D11DeviceContext* ctx)
	{
		ctx->ClearDepthStencilView(m_depthstencilView.Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	}

	void Renderer::CopyDepthFrom(ID3D11DeviceContext* ctx, ID3D11Texture2D* srcDepth)
	{
		if (srcDepth && m_depthTexture)
			ctx->CopyResource(m_depthTexture.Get(), srcDepth);
	}

}
