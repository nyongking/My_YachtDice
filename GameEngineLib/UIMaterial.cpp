#include "GameEnginePch.h"
#include "UIMaterial.h"

namespace GameEngine
{
	UIMaterial::UIMaterial(const UIMaterial& rhs)
		: Material(rhs)
		, m_uiData(rhs.m_uiData)
		, m_uvRectData(rhs.m_uvRectData)
	{
	}

	bool UIMaterial::Initialize(Render::ShaderGroup* shaderGroup, ID3D11Device* device)
	{
		if (!shaderGroup || !device)
			return false;

		m_shaderGroup = shaderGroup;
		m_device      = device;

		AutomaticRegisterPS("UIData", "UIData", &m_uiData, shaderGroup);
		AutomaticRegisterVS("UVRect", "UVRect", &m_uvRectData, shaderGroup);
		AutomaticRegisterPSTexture("UITexture", "gTexture", shaderGroup);

		return true;
	}

	std::unique_ptr<Render::Material> UIMaterial::Clone() const
	{
		auto clone = std::make_unique<UIMaterial>(*this);
		clone->ChangeParameterAddress(0, &clone->m_uiData);
		clone->ChangeParameterAddress(1, &clone->m_uvRectData);
		return clone;
	}
}
