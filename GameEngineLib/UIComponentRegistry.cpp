#include "GameEnginePch.h"
#include "UIComponentRegistry.h"
#include "UIComponent.h"

namespace GameEngine
{
	std::unordered_map<std::string, UIComponentRegistry::FactoryFn>& UIComponentRegistry::GetRegistry()
	{
		static std::unordered_map<std::string, FactoryFn> s_registry;
		return s_registry;
	}

	void UIComponentRegistry::Register(const std::string& typeName, FactoryFn factory)
	{
		GetRegistry()[typeName] = std::move(factory);
	}

	std::unique_ptr<UIComponent> UIComponentRegistry::Create(const std::string& typeName)
	{
		auto& registry = GetRegistry();
		auto  it       = registry.find(typeName);
		if (it == registry.end())
			return nullptr;
		return it->second();
	}
}
