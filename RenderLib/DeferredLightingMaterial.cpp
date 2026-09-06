#include "RenderPch.h"
#include "DeferredLightingMaterial.h"

namespace Render
{
	DeferredLightingMaterial::DeferredLightingMaterial(const DeferredLightingMaterial& rhs)
		: Material(rhs)
		, m_lightBuffer(rhs.m_lightBuffer)
	{
	}

	bool DeferredLightingMaterial::Initialize(ShaderGroup* shaderGroup, ID3D11Device* device)
	{
		if (!shaderGroup || !device)
			return false;

		m_shaderGroup = shaderGroup;
		m_device      = device;

		// PS: G-Buffer SRV 슬롯 등록 (LightingPass::Execute()에서 SetTexture로 채워짐)
		AutomaticRegisterPSTexture("gAlbedo",   "gAlbedo",   shaderGroup);
		AutomaticRegisterPSTexture("gNormal",   "gNormal",   shaderGroup);
		AutomaticRegisterPSTexture("gWorldPos", "gWorldPos", shaderGroup);

		// PS: Light 배열 CB (b0)
		AutomaticRegisterPS("LightBuffer", "LightBuffer", &m_lightBuffer, shaderGroup);

		return true;
	}

	std::unique_ptr<Material> DeferredLightingMaterial::Clone() const
	{
		auto clone = std::make_unique<DeferredLightingMaterial>(*this);
		clone->ChangeParameterAddress(0, &clone->m_lightBuffer);
		return clone;
	}

	void DeferredLightingMaterial::SetLights(const LightData* lights, int count)
	{
		m_lightBuffer.lightCount = count < MAX_LIGHTS ? count : MAX_LIGHTS;
		for (int i = 0; i < m_lightBuffer.lightCount; ++i)
			m_lightBuffer.lights[i] = lights[i];
		MarkParameterDirty(0);
	}

	void DeferredLightingMaterial::SetCameraPosition(const float3& pos)
	{
		m_lightBuffer.cameraPos[0] = pos.x;
		m_lightBuffer.cameraPos[1] = pos.y;
		m_lightBuffer.cameraPos[2] = pos.z;
		MarkParameterDirty(0);
	}
}
