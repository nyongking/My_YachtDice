#pragma once
#include "Material.h"

namespace GameEngine
{
	class WireframeMaterial : public Render::Material
	{
	public:
		bool Initialize(Render::ShaderGroup* sg, ID3D11Device* dev) override
		{
			m_shaderGroup = sg;
			m_device      = dev;
			return true;
		}

		std::unique_ptr<Render::Material> Clone() const override
		{
			return std::make_unique<WireframeMaterial>(*this);
		}
	};
}
