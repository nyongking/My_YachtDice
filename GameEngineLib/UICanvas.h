#pragma once
#include "RenderTypes.h"
#include "UIComponent.h"
#include "UIAnimation.h"
#include "UIPanel.h"
#include <vector>
#include <memory>
#include <string>

namespace GameEngine
{
	class UICanvas
	{
	public:
		explicit UICanvas(const std::string& name);

		const std::string& GetName() const { return m_name; }
		void SetName(const std::string& name) { m_name = name; }

		void   SetPosition(const float2& pos) { m_position = pos; }
		float2 GetPosition() const            { return m_position; }

		void SetPositionScreen(const float2& screenPos);

		void   SetSize(const float2& size) { m_size = size; }
		float2 GetSize() const             { return m_size; }
		float2 GetScaledSize() const       { return m_scaledSize; }

		void  SetDepth(float depth) { m_depth = depth; }
		float GetDepth() const      { return m_depth; }

		void SetVisible(bool v) { m_visible = v; }
		bool IsVisible() const  { return m_visible; }

		void SetAnimation(UIAnimation::Type type, float amplitude = 0.1f, float speed = 3.f);
		void ClearAnimation();

		// Panel (background)
		void SetPanel(std::unique_ptr<UIPanel> panel);
		UIPanel* GetPanel() const { return m_panel.get(); }
		void RemovePanel();

		// Child components
		UIComponent* AddComponent(std::unique_ptr<UIComponent> comp);
		void         RemoveComponent(UIComponent* comp);
		UIComponent* FindComponent(const std::string& name) const;
		int          GetComponentCount() const { return static_cast<int>(m_components.size()); }
		UIComponent* GetComponent(int index) const { return m_components[index].get(); }

		float2   GetScreenPosition() const;
		float    GetResolutionScale() const;
		float4x4 GetOrthoViewProj() const;

		void Initialize();
		void SubmitUI(float dt);

		MyJson Serialize() const;
		void   Deserialize(const MyJson& j);

	private:
		std::string m_name;
		float2      m_position   = { 0.f, 0.f };
		float2      m_size       = { 0.f, 0.f };
		float2      m_scaledSize = { 0.f, 0.f };
		float       m_depth      = 0.f;
		bool        m_visible    = true;
		UIAnimation m_animation;

		std::unique_ptr<UIPanel>                      m_panel;
		std::vector<std::unique_ptr<UIComponent>>     m_components;
		bool        m_componentsDirty = false;

		void SortComponentsByDepth();
	};
}
