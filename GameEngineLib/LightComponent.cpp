#include "GameEnginePch.h"
#include "LightComponent.h"

#include "RenderPipeline.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#endif

namespace GameEngine
{
#ifdef _DEBUG
	void LightComponent::OnInspectorGUI()
	{
		const char* types[] = { "Directional", "Point", "Spot" };
		int typeInt = static_cast<int>(m_data.type);
		if (ImGui::Combo("Type", &typeInt, types, 3))
			m_data.type = static_cast<unsigned int>(typeInt);

		ImGui::ColorEdit3("Color",    &m_data.color.x);
		ImGui::DragFloat("Intensity", &m_data.intensity, 0.01f, 0.f, 100.f);

		if (typeInt != 0)  // Point, Spot
			ImGui::DragFloat("Range", &m_data.range, 0.1f, 0.f, 10000.f);

		if (typeInt != 1)  // Directional, Spot
			ImGui::DragFloat3("Direction", &m_data.direction.x, 0.01f);

		if (typeInt == 2)  // Spot
		{
			ImGui::DragFloat("Spot Inner", &m_data.spotInner, 0.001f, 0.f, 1.f);
			ImGui::DragFloat("Spot Outer", &m_data.spotOuter, 0.001f, 0.f, 1.f);
		}
	}
#endif

	void LightComponent::LateUpdate(float dt)
	{
		Render::RenderPipeline::GetInstance().SubmitLight({ m_data });
	}

	static const char* s_lightTypeNames[] = { "Directional", "Point", "Spot" };

	MyJson LightComponent::Serialize() const
	{
		MyJson j;
		j["type"]      = GetTypeName();
		j["lightType"] = s_lightTypeNames[m_data.type];
		j["color"]     = { m_data.color.x,     m_data.color.y,     m_data.color.z     };
		j["direction"] = { m_data.direction.x, m_data.direction.y, m_data.direction.z };
		j["position"]  = { m_data.position.x,  m_data.position.y,  m_data.position.z  };
		j["intensity"] = m_data.intensity;
		j["range"]     = m_data.range;
		j["spotInner"] = m_data.spotInner;
		j["spotOuter"] = m_data.spotOuter;
		return j;
	}

	void LightComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("lightType"))
		{
			std::string lt = j["lightType"].get<std::string>();
			if      (lt == "Point") m_data.type = static_cast<unsigned int>(Render::LightType::Point);
			else if (lt == "Spot")  m_data.type = static_cast<unsigned int>(Render::LightType::Spot);
			else                    m_data.type = static_cast<unsigned int>(Render::LightType::Directional);
		}
		if (j.contains("color"))
			m_data.color = { j["color"][0], j["color"][1], j["color"][2] };
		if (j.contains("direction"))
			m_data.direction = { j["direction"][0], j["direction"][1], j["direction"][2] };
		if (j.contains("position"))
			m_data.position  = { j["position"][0],  j["position"][1],  j["position"][2]  };

		if (j.contains("intensity")) m_data.intensity = j["intensity"];
		if (j.contains("range"))     m_data.range     = j["range"];
		if (j.contains("spotInner")) m_data.spotInner = j["spotInner"];
		if (j.contains("spotOuter")) m_data.spotOuter = j["spotOuter"];
	}
}
