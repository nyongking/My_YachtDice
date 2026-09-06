#include "GameEnginePch.h"
#include "QuadComponent.h"
#include "GeometryManager.h"

namespace GameEngine
{
	void QuadComponent::Awake()
	{
		Render::Geometry* quad = GeometryManager::GetInstance()->Get("Quad");
		assert(quad && "Quad geometry not loaded — InitEngine() 호출 확인");
		SetGeometry(quad);
	}

}
