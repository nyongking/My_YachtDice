#pragma once
#include "Material.h"
#include "LightItem.h"

namespace Render
{
	// Lighting Pass용 Material
	// G-Buffer SRV 3개 + LightData 배열 CB → backbuffer에 조명 출력
	class DeferredLightingMaterial : public Material
	{
	public:
		DeferredLightingMaterial()  = default;
		~DeferredLightingMaterial() = default;

		DeferredLightingMaterial(const DeferredLightingMaterial& rhs);

	public:
		bool Initialize(ShaderGroup* shaderGroup, ID3D11Device* device) override;
		std::unique_ptr<Material> Clone() const override;

		void SetLights(const LightData* lights, int count) override;
		void SetCameraPosition(const float3& pos);

	private:
		struct alignas(16) LightBufferData
		{
			int       lightCount        = 0;
			float     cameraPos[3]      = {};
			LightData lights[MAX_LIGHTS] = {};
		};

		LightBufferData m_lightBuffer = {};
	};
}
