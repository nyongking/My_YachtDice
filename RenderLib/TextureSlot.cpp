#include "RenderPch.h"
#include "TextureSlot.h"

namespace Render
{
	TextureSlot::TextureSlot(std::string name, int slotVS, int slotPS)
		: m_name(std::move(name))
		, m_slotVS(slotVS)
		, m_slotPS(slotPS)
	{
	}

	void TextureSlot::Bind(ID3D11DeviceContext* ctx)
	{
		if (!ctx)
			return;

		if (m_slotVS >= 0)
			ctx->VSSetShaderResources(static_cast<UINT>(m_slotVS), 1, &m_srv);

		if (m_slotPS >= 0)
			ctx->PSSetShaderResources(static_cast<UINT>(m_slotPS), 1, &m_srv);
	}
}
