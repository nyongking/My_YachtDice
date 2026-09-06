#pragma once
#include "RenderComponent.h"
#include "Material.h"

namespace GameEngine
{
	class QuadComponent : public RenderComponent
	{
	public:
		void Awake() override;

		std::string GetTypeName() const override { return "QuadComponent"; }
	};


}

