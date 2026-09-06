#include "GameEnginePch.h"
#include "ComponentRegistry.h"
#include "Component.h"

namespace GameEngine
{
	std::unordered_map<std::string, ComponentRegistry::FactoryFn>& ComponentRegistry::GetRegistry()
	{
		static std::unordered_map<std::string, FactoryFn> s_registry;
		return s_registry;
	}

	void ComponentRegistry::Register(const std::string& typeName, FactoryFn factory)
	{
		GetRegistry()[typeName] = std::move(factory);
	}

	std::unique_ptr<Component> ComponentRegistry::Create(const std::string& typeName)
	{
		auto& registry = GetRegistry();
		auto  it       = registry.find(typeName);
		if (it == registry.end())
		{
			// Unregistered type -- call Register() after InitEngine()
			assert(false && "ComponentRegistry: unregistered component type");
			return nullptr;
		}
		return it->second();
	}
}
