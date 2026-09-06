#include "RenderPch.h"
#include "Texture.h"

#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"

#ifdef _DEBUG
#pragma comment(lib, ".\\Lib\\Debug\\DirectXTKd.lib")
#else
#pragma comment(lib, ".\\Lib\\Release\\DirectXTK.lib")
#endif

namespace Render
{
	bool Texture::IsWicFormat(const std::wstring& path)
	{
		auto dot = path.rfind(L'.');
		if (dot == std::wstring::npos)
			return true;

		std::wstring ext = path.substr(dot);
		for (auto& c : ext)
			c = static_cast<wchar_t>(std::tolower(c));

		return ext != L".dds";
	}

	std::unique_ptr<Texture> Texture::LoadFromFile(
		ID3D11Device*       device,
		const std::wstring& path)
	{
		if (!device || path.empty())
			return nullptr;

		auto tex = std::make_unique<Texture>();

		HRESULT hr;
		if (IsWicFormat(path))
		{
			hr = DirectX::CreateWICTextureFromFile(
				device,
				path.c_str(),
				tex->m_resource.GetAddressOf(),
				tex->m_srv.GetAddressOf());
		}
		else
		{
			hr = DirectX::CreateDDSTextureFromFile(
				device,
				path.c_str(),
				tex->m_resource.GetAddressOf(),
				tex->m_srv.GetAddressOf());
		}

		if (FAILED(hr))
		{
#ifdef _DEBUG
			OutputDebugStringA("Texture::LoadFromFile - Failed\n");
#endif
			return nullptr;
		}

		// 텍스처 크기 추출
		RefCom<ID3D11Texture2D> tex2D;
		if (SUCCEEDED(tex->m_resource.As(&tex2D)))
		{
			D3D11_TEXTURE2D_DESC desc = {};
			tex2D->GetDesc(&desc);
			tex->m_width  = desc.Width;
			tex->m_height = desc.Height;
		}

		return tex;
	}

	void Texture::InitFromRaw(ID3D11Resource* res, ID3D11ShaderResourceView* srv, UINT w, UINT h)
	{
		m_resource = res;
		m_srv      = srv;
		m_width    = w;
		m_height   = h;
	}
}
