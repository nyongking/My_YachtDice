#pragma once

namespace GameEngine
{
	class Component;

	// 타입명 문자열 → Component 생성 팩토리 레지스트리.
	// InitEngine() 이후, 씬 로드 이전에 Register를 완료해야 한다.
	class ComponentRegistry
	{
	public:
		using FactoryFn = std::function<std::unique_ptr<Component>()>;

		// Register<T>("TypeName") — 대부분의 경우 이 오버로드를 사용
		template<typename T>
		static void Register(const std::string& typeName)
		{
			Register(typeName, [] { return std::make_unique<T>(); });
		}

		// 커스텀 팩토리가 필요한 경우
		static void Register(const std::string& typeName, FactoryFn factory);

		// typeName에 해당하는 Component 인스턴스를 생성해서 반환.
		// 미등록 타입이면 nullptr 반환.
		static std::unique_ptr<Component> Create(const std::string& typeName);

	private:
		static std::unordered_map<std::string, FactoryFn>& GetRegistry();
	};
}
