#include "GameEnginePch.h"
#include "UICanvas.h"
#include "UIComponentRegistry.h"
#include "UIManager.h"
#include "Renderer.h"

using namespace DirectX;

namespace GameEngine
{
	namespace
	{
		constexpr float UI_REFERENCE_WIDTH  = 1600.f;
		constexpr float UI_REFERENCE_HEIGHT = 900.f;
	}

	UICanvas::UICanvas(const std::string& name)
		: m_name(name)
	{
	}

	void UICanvas::SetPanel(std::unique_ptr<UIPanel> panel)
	{
		m_panel = std::move(panel);
	}

	void UICanvas::RemovePanel()
	{
		m_panel.reset();
	}

	UIComponent* UICanvas::AddComponent(std::unique_ptr<UIComponent> comp)
	{
		if (!comp) return nullptr;
		comp->SetCanvasContext(this);
		auto* raw = comp.get();
		m_components.push_back(std::move(comp));
		m_componentsDirty = true;
		return raw;
	}

	void UICanvas::RemoveComponent(UIComponent* comp)
	{
		UIManager::GetInstance().NotifyComponentRemoved(comp);
		auto it = std::remove_if(m_components.begin(), m_components.end(),
			[comp](const std::unique_ptr<UIComponent>& p) { return p.get() == comp; });
		m_components.erase(it, m_components.end());
	}

	UIComponent* UICanvas::FindComponent(const std::string& name) const
	{
		for (auto& comp : m_components)
		{
			if (comp->GetName() == name)
				return comp.get();
		}
		return nullptr;
	}

	float2 UICanvas::GetScreenPosition() const
	{
		float w = static_cast<float>(Render::Renderer::GetInstance().GetWidth());
		float h = static_cast<float>(Render::Renderer::GetInstance().GetHeight());
		return { m_position.x * w, m_position.y * h };
	}

	float UICanvas::GetResolutionScale() const
	{
		float w = static_cast<float>(Render::Renderer::GetInstance().GetWidth());
		float h = static_cast<float>(Render::Renderer::GetInstance().GetHeight());
		float widthScale = w / UI_REFERENCE_WIDTH;
		float heightScale = h / UI_REFERENCE_HEIGHT;
		return widthScale < heightScale ? widthScale : heightScale;
	}

	void UICanvas::SetPositionScreen(const float2& screenPos)
	{
		float w = static_cast<float>(Render::Renderer::GetInstance().GetWidth());
		float h = static_cast<float>(Render::Renderer::GetInstance().GetHeight());
		m_position = { screenPos.x / w, screenPos.y / h };
	}

	float4x4 UICanvas::GetOrthoViewProj() const
	{
		float w = static_cast<float>(Render::Renderer::GetInstance().GetWidth());
		float h = static_cast<float>(Render::Renderer::GetInstance().GetHeight());

		XMMATRIX ortho = XMMatrixOrthographicOffCenterLH(0.f, w, 0.f, h, 0.f, 1000.f);
		float4x4 result;
		XMStoreFloat4x4(&result, ortho);
		return result;
	}

	void UICanvas::Initialize()
	{
		if (m_panel)
			m_panel->Initialize();

		for (auto& comp : m_components)
			comp->Initialize();
	}

	void UICanvas::SetAnimation(UIAnimation::Type type, float amplitude, float speed)
	{
		m_animation.type      = type;
		m_animation.amplitude = amplitude;
		m_animation.speed     = speed;
		m_animation.Reset();
	}

	void UICanvas::ClearAnimation()
	{
		m_animation.type = UIAnimation::Type::None;
		m_animation.Reset();
	}

	void UICanvas::SortComponentsByDepth()
	{
		if (!m_componentsDirty)
			return;

		std::stable_sort(m_components.begin(), m_components.end(),
			[](const std::unique_ptr<UIComponent>& a, const std::unique_ptr<UIComponent>& b)
			{
				return a->GetDepth() > b->GetDepth();
			});

		m_componentsDirty = false;
	}

	void UICanvas::SubmitUI(float dt)
	{
		if (!m_visible)
			return;

		SortComponentsByDepth();

		float scale = m_animation.Update(dt) * GetResolutionScale();
		m_scaledSize = { m_size.x * scale, m_size.y * scale };

		float4x4 orthoVP = GetOrthoViewProj();
		float screenH = static_cast<float>(Render::Renderer::GetInstance().GetHeight());
		float2 pixelPos = GetScreenPosition();

		if (m_panel)
		{
			float2 renderPos = { pixelPos.x, screenH - pixelPos.y };
			m_panel->SubmitUI(renderPos, m_scaledSize, m_depth, orthoVP);
		}

		for (auto& comp : m_components)
			comp->SubmitUI(dt);
	}

	MyJson UICanvas::Serialize() const
	{
		MyJson j;
		j["name"]    = m_name;
		j["posX"]    = m_position.x;
		j["posY"]    = m_position.y;
		j["width"]   = m_size.x;
		j["height"]  = m_size.y;
		j["depth"]   = m_depth;
		j["visible"] = m_visible;
		if (m_animation.type != UIAnimation::Type::None)
			j["animation"] = m_animation.Serialize();

		if (m_panel)
			j["panel"] = m_panel->Serialize();

		MyJson comps = MyJson::array();
		for (auto& comp : m_components)
			comps.push_back(comp->Serialize());
		j["components"] = comps;

		return j;
	}

	void UICanvas::Deserialize(const MyJson& j)
	{
		if (j.contains("name"))
			m_name = j["name"].get<std::string>();
		if (j.contains("posX"))
			m_position.x = j["posX"].get<float>();
		if (j.contains("posY"))
			m_position.y = j["posY"].get<float>();
		if (j.contains("width"))
			m_size.x = j["width"].get<float>();
		if (j.contains("height"))
			m_size.y = j["height"].get<float>();
		if (j.contains("depth"))
			m_depth = j["depth"].get<float>();
		if (j.contains("visible"))
			m_visible = j["visible"].get<bool>();
		if (j.contains("animation"))
			m_animation.Deserialize(j["animation"]);

		m_scaledSize = m_size;

		if (j.contains("panel"))
		{
			auto panel = std::make_unique<UIPanel>();
			panel->Initialize();
			panel->Deserialize(j["panel"]);
			SetPanel(std::move(panel));
		}

		if (j.contains("components"))
		{
			for (auto& cj : j["components"])
			{
				std::string typeName = cj.value("type", "");
				auto comp = UIComponentRegistry::Create(typeName);
				if (comp)
				{
					comp->Initialize();
					comp->Deserialize(cj);
					AddComponent(std::move(comp));
				}
			}
		}
	}
}
