#include "GameEnginePch.h"
#include "GameObject.h"
#include "ComponentRegistry.h"

namespace GameEngine
{
	GameObject::GameObject()
	{
		m_transform          = std::make_unique<Transform>();
		m_transform->m_owner = this;
	}

	Component* GameObject::AddComponent(std::unique_ptr<Component> comp)
	{
		comp->m_owner = this;
		comp->Awake();
		Component* raw = comp.get();
		m_componentMap.emplace(typeid(*comp), raw);
		m_components.push_back(std::move(comp));
		return raw;
	}

	MyJson GameObject::Serialize() const
	{
		MyJson j;
		j["name"]      = m_name;
		j["active"]    = m_active;
		j["transform"] = m_transform->Serialize();

		MyJson compsArray = MyJson::array();
		for (const auto& comp : m_components)
		{
			MyJson compJson = comp->Serialize();
			if (!compJson.empty())
				compsArray.push_back(compJson);
		}
		j["components"] = compsArray;

		return j;
	}

	void GameObject::Deserialize(const MyJson& j)
	{
		if (j.contains("name"))      m_name   = j["name"].get<std::string>();
		if (j.contains("active"))    m_active  = j["active"].get<bool>();
		if (j.contains("transform")) m_transform->Deserialize(j["transform"]);

		if (!j.contains("components")) return;

		for (const auto& compJson : j["components"])
		{
			if (!compJson.contains("type")) continue;

			std::string typeName = compJson["type"].get<std::string>();
			auto comp = ComponentRegistry::Create(typeName);
			if (!comp) continue;

			// Awake() 먼저 (기본값 세팅) → 이후 Deserialize로 저장 값 덮어쓰기
			Component* raw = AddComponent(std::move(comp));
			raw->Deserialize(compJson);
		}
	}

	void GameObject::Start()
	{
		for (auto& comp : m_components)
			comp->Start();
	}

	void GameObject::Update(float dt)
	{
		if (!m_active)
			return;

		for (auto& comp : m_components)
			comp->Update(dt);
	}

	void GameObject::LateUpdate(float dt)
	{
		if (!m_active)
			return;

		for (auto& comp : m_components)
			comp->LateUpdate(dt);
	}
}
