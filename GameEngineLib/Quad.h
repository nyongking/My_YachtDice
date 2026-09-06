#pragma once
#include "Geometry.h"

namespace GameEngine
{
	class Quad : public Render::Geometry
	{
	public:
		Quad() = default;
		virtual ~Quad() = default;

	public:
		virtual bool DefaultCreateBuffers(ID3D11Device* pDevice) override;
	};

}


