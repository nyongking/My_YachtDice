#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace GameEngine
{
	class UIComponent;

	class UIComponentRegistry
	{
	public:
		using FactoryFn = std::function<std::unique_ptr<UIComponent>()>;

		template<typename T>
		static void Register(const std::string& typeName)
		{
			Register(typeName, [] { return std::make_unique<T>(); });
		}

		static void Register(const std::string& typeName, FactoryFn factory);
		static std::unique_ptr<UIComponent> Create(const std::string& typeName);

	private:
		static std::unordered_map<std::string, FactoryFn>& GetRegistry();
	};
}
