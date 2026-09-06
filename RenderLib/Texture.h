#pragma once

namespace Render
{
	class Texture
	{
	public:
		static std::unique_ptr<Texture> LoadFromFile(
			ID3D11Device*       device,
			const std::wstring& path);

		ID3D11ShaderResourceView* SRV()      const { return m_srv.Get(); }
		bool                      IsLoaded() const { return m_srv != nullptr; }
		UINT                      GetWidth()  const { return m_width; }
		UINT                      GetHeight() const { return m_height; }

		// 외부에서 생성한 리소스로 초기화
		void InitFromRaw(ID3D11Resource* res, ID3D11ShaderResourceView* srv, UINT w, UINT h);

	private:
		static bool IsWicFormat(const std::wstring& path);

		RefCom<ID3D11Resource>           m_resource;
		RefCom<ID3D11ShaderResourceView> m_srv;
		UINT                             m_width  = 0;
		UINT                             m_height = 0;
	};
}
