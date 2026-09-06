#pragma once

namespace Render
{
	class TextureSlot
	{
	public:
		TextureSlot(std::string name, int slotVS, int slotPS);
		TextureSlot(const TextureSlot&) = default;

	public:
		const std::string& Name() const { return m_name; }
		void SetSRV(ID3D11ShaderResourceView* srv) { m_srv = srv; }
		void Bind(ID3D11DeviceContext* ctx);

	private:
		std::string               m_name;
		ID3D11ShaderResourceView* m_srv    = nullptr;  // 비소유, TextureManager가 소유
		int                       m_slotVS = -1;
		int                       m_slotPS = -1;
	};
}
