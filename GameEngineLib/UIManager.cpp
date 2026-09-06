#include "GameEnginePch.h"
#include "UIManager.h"
#include "UICanvas.h"
#include "InputManager.h"
#include <algorithm>

namespace GameEngine
{
	UICanvas* UIManager::CreateCanvas(const std::string& name, bool global)
	{
		auto canvas = std::make_unique<UICanvas>(name);
		auto* raw = canvas.get();
		m_canvases.push_back({ std::move(canvas), global });
		m_dirty = true;
		return raw;
	}

	void UIManager::RemoveCanvas(const std::string& name)
	{
		auto it = std::remove_if(m_canvases.begin(), m_canvases.end(),
			[&name](const CanvasEntry& e) { return e.canvas->GetName() == name; });
		m_canvases.erase(it, m_canvases.end());
		m_hoveredComponent = nullptr;
		m_pressedComponent = nullptr;
	}

	UICanvas* UIManager::FindCanvas(const std::string& name) const
	{
		for (auto& entry : m_canvases)
		{
			if (entry.canvas->GetName() == name)
				return entry.canvas.get();
		}
		return nullptr;
	}

	void UIManager::SetCanvasVisible(const std::string& name, bool v)
	{
		auto* canvas = FindCanvas(name);
		if (canvas)
			canvas->SetVisible(v);
	}

	void UIManager::SetCanvasPosition(const std::string& name, const float2& normalized)
	{
		auto* canvas = FindCanvas(name);
		if (canvas)
			canvas->SetPosition(normalized);
	}

	void UIManager::SetCanvasPositionScreen(const std::string& name, const float2& screenPos)
	{
		auto* canvas = FindCanvas(name);
		if (canvas)
			canvas->SetPositionScreen(screenPos);
	}

	void UIManager::ClearSceneCanvases()
	{
		auto it = std::remove_if(m_canvases.begin(), m_canvases.end(),
			[](const CanvasEntry& e) { return !e.global; });
		m_canvases.erase(it, m_canvases.end());
		m_hoveredComponent = nullptr;
		m_pressedComponent = nullptr;
	}

	void UIManager::ClearAll()
	{
		m_canvases.clear();
		m_hoveredComponent = nullptr;
		m_pressedComponent = nullptr;
	}

	void UIManager::SortByDepth()
	{
		if (!m_dirty)
			return;

		std::stable_sort(m_canvases.begin(), m_canvases.end(),
			[](const CanvasEntry& a, const CanvasEntry& b)
			{
				return a.canvas->GetDepth() > b.canvas->GetDepth();
			});

		m_dirty = false;
	}

	void UIManager::SubmitUI(float dt)
	{
		ProcessInteraction();
		SortByDepth();

		for (auto& entry : m_canvases)
			entry.canvas->SubmitUI(dt);
	}

	void UIManager::ProcessInteraction()
	{
		SortByDepth();

		auto& input = InputManager::GetInstance();
		float2 mousePos  = input.GetMousePosition();
		bool lmbPressed  = input.IsMouseButtonPressed(0);
		bool lmbDown     = input.IsMouseButtonDown(0);
		bool lmbReleased = input.IsMouseButtonReleased(0);

		UIComponent* hit = HitTestAll(mousePos);

		// Hover transitions
		if (hit != m_hoveredComponent)
		{
			if (m_hoveredComponent && m_hoveredComponent != m_pressedComponent)
			{
				m_hoveredComponent->m_interactionState = UIInteractionState::None;
				m_hoveredComponent->OnHoverExit();
				if (m_hoveredComponent->m_onHoverExit)
					m_hoveredComponent->m_onHoverExit(m_hoveredComponent);
			}
			if (hit && !m_pressedComponent)
			{
				hit->m_interactionState = UIInteractionState::Hovered;
				hit->OnHoverEnter();
				if (hit->m_onHoverEnter)
					hit->m_onHoverEnter(hit);
			}
			m_hoveredComponent = hit;
		}

		// Press
		if (lmbPressed && hit)
		{
			m_pressedComponent = hit;
			hit->m_interactionState = UIInteractionState::Pressed;
			hit->OnClick();
			if (hit->m_onClick) hit->m_onClick(hit);
		}
		// Held
		else if (lmbDown && m_pressedComponent)
		{
			m_pressedComponent->m_interactionState = UIInteractionState::Held;
			m_pressedComponent->OnHeld();
			if (m_pressedComponent->m_onHeld)
				m_pressedComponent->m_onHeld(m_pressedComponent);
		}
		// Release
		else if (lmbReleased && m_pressedComponent)
		{
			if (hit == m_pressedComponent)
			{
				m_pressedComponent->m_interactionState = UIInteractionState::Released;
				m_pressedComponent->OnRelease();
				if (m_pressedComponent->m_onRelease)
					m_pressedComponent->m_onRelease(m_pressedComponent);
				m_pressedComponent->m_interactionState = UIInteractionState::Hovered;
			}
			else
			{
				m_pressedComponent->m_interactionState = UIInteractionState::None;
			}
			m_pressedComponent = nullptr;
		}
		// Idle hover
		else if (hit && !m_pressedComponent)
		{
			hit->m_interactionState = UIInteractionState::Hovered;
		}
	}

	UIComponent* UIManager::HitTestAll(const float2& mousePos) const
	{
		for (auto& entry : m_canvases)
		{
			if (!entry.canvas->IsVisible()) continue;

			int count = entry.canvas->GetComponentCount();
			for (int i = 0; i < count; ++i)
			{
				UIComponent* comp = entry.canvas->GetComponent(i);
				if (comp->HitTest(mousePos))
					return comp;
			}
		}
		return nullptr;
	}

	void UIManager::NotifyComponentRemoved(UIComponent* comp)
	{
		if (m_hoveredComponent == comp) m_hoveredComponent = nullptr;
		if (m_pressedComponent == comp) m_pressedComponent = nullptr;
	}

	void UIManager::LoadSingleCanvasFromJson(const MyJson& j, bool global)
	{
		std::string name = j.value("name", "");
		bool isGlobal = j.value("global", global);

		auto canvas = std::make_unique<UICanvas>(name);
		canvas->Deserialize(j);

		m_canvases.push_back({ std::move(canvas), isGlobal });
		m_dirty = true;
	}

	void UIManager::LoadCanvasesFromJson(const MyJson& arr, bool global)
	{
		for (auto& cj : arr)
		{
			if (cj.is_string())
			{
				// External file reference
				MyJson ext;
				if (!LoadJson(cj.get<std::string>().c_str(), ext))
					continue;

				if (ext.contains("canvases"))
					LoadCanvasesFromJson(ext["canvases"], global);
				else if (ext.contains("components"))
					LoadSingleCanvasFromJson(ext, global);
			}
			else
			{
				std::string name = cj.value("name", "");
				bool isGlobal = cj.value("global", global);

				auto canvas = std::make_unique<UICanvas>(name);
				canvas->Deserialize(cj);

				m_canvases.push_back({ std::move(canvas), isGlobal });
			}
		}
		m_dirty = true;
	}
}
