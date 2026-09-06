#pragma once

#include "Component.h"
#include "Transform.h"
#include "ComponentRegistry.h"

namespace GameEngine
{
	class GameObject
	{
	public:
		GameObject();
		~GameObject() = default;

		template<typename T, typename... Args>
		T* AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of<Component, T>::value,
				"T must derive from Component");

			auto comp        = std::make_unique<T>(std::forward<Args>(args)...);
			T*   raw         = comp.get();
			comp->m_owner    = this;
			comp->Awake();
			m_componentMap.emplace(typeid(T), raw);   // first-wins: 중복 시 첫 번째 유지
			m_components.push_back(std::move(comp));
			return raw;
		}

		template<typename T>
		T* GetComponent() const
		{
			static_assert(std::is_base_of<Component, T>::value,
				"T must derive from Component");

			auto it = m_componentMap.find(typeid(T));
			if (it != m_componentMap.end())
				return static_cast<T*>(it->second);
			return nullptr;
		}

		template<>
		Transform* GetComponent<Transform>() const
		{
			return m_transform.get();
		}

		// 팩토리에서 생성된 Component를 동적으로 추가 (역직렬화용).
		// Awake()를 호출하고, 호출자가 이후 Deserialize()를 적용할 수 있도록 raw ptr 반환.
		Component* AddComponent(std::unique_ptr<Component> comp);

		void Start();
		void Update(float dt);
		void LateUpdate(float dt);

		// 직렬화
		MyJson Serialize()              const;
		void   Deserialize(const MyJson& j);

		Transform*         GetTransform() const { return m_transform.get(); }

		void ForEachComponent(const std::function<void(Component*)>& fn) const
		{
			for (auto& comp : m_components) fn(comp.get());
		}
		const std::string& GetName()      const { return m_name; }
		void               SetName(const std::string& name) { m_name = name; }
		bool               IsActive()     const { return m_active; }
		void               SetActive(bool active) { m_active = active; }

	private:
		std::string m_name   = "GameObject";
		bool        m_active = true;

		std::unique_ptr<Transform>              m_transform;
		std::vector<std::unique_ptr<Component>> m_components;
		std::unordered_map<std::type_index, Component*> m_componentMap;
	};
}
