#include "RenderPch.h"
#include "GBufferMaterial.h"

namespace Render
{
	GBufferMaterial::GBufferMaterial(const GBufferMaterial& rhs)
		: Material(rhs)
		, m_materialData(rhs.m_materialData)
	{
	}

	bool GBufferMaterial::Initialize(ShaderGroup* shaderGroup, ID3D11Device* device)
	{
		if (!shaderGroup || !device)
			return false;

		m_shaderGroup = shaderGroup;
		m_device      = device;

		// PS: MaterialData CB (b0) — albedo 색상 + UV tiling/offset
		AutomaticRegisterPS("MaterialData", "MaterialData", &m_materialData, shaderGroup);

		// PS: Albedo 텍스처 슬롯 등록
		AutomaticRegisterPSTexture("AlbedoMap", "gAlbedoTex", shaderGroup);

		// PS: Normal 텍스처 슬롯 등록
		AutomaticRegisterPSTexture("NormalMap", "gNormalTex", shaderGroup);

		return true;
	}

	std::unique_ptr<Material> GBufferMaterial::Clone() const
	{
		auto clone = std::make_unique<GBufferMaterial>(*this);
		clone->ChangeParameterAddress(0, &clone->m_materialData);
		return clone;
	}
}
