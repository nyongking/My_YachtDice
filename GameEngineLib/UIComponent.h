#pragma once
#include "Material.h"
#include "UIAnimation.h"
#include <memory>
#include <string>
#include <functional>

using MyJson = nlohmann::json;

namespace Render { class Geometry; class Material; class Texture; }

namespace GameEngine
{
	enum class UIAnchor
	{
		TopLeft, Top, TopRight,
		Left, Center, Right,
		BottomLeft, Bottom, BottomRight
	};

	enum class UIInteractionState
	{
		None,
		Hovered,
		Pressed,
		Held,
		Released
	};

	class UICanvas;
	class UIManager;

	class UIComponent
	{
		friend class UIManager;
	public:
		using UICallback = std::function<void(UIComponent*)>;

		virtual ~UIComponent();

		void SetName(const std::string& name) { m_name = name; }
		const std::string& GetName() const    { return m_name; }

		void SetAnchor(UIAnchor anchor)       { m_anchor = anchor; }
		UIAnchor GetAnchor() const            { return m_anchor; }

		void SetPosition(const float2& pos)   { m_position = pos; }
		float2 GetPosition() const            { return m_position; }

		void SetPositionX(float x) { m_position.x = x; }
		void SetPositionY(float y) { m_position.y = y; }

		void SetSize(const float2& size)       { m_size = size; m_dirty = true; }
		float2 GetSize() const                 { return m_size; }

		void SetDepth(float depth)             { m_depth = depth; }
		float GetDepth() const                 { return m_depth; }

		void SetRotation(float radians)        { m_rotation = radians; }
		float GetRotation() const              { return m_rotation; }

		void SetPivot(const float2& pivot)     { m_pivot = pivot; }
		float2 GetPivot() const                { return m_pivot; }

		void SetCanvasContext(UICanvas* canvas) { m_canvas = canvas; }
		UICanvas* GetCanvas() const            { return m_canvas; }

		void SetAnimation(UIAnimation::Type type, float amplitude = 0.1f, float speed = 3.f);
		void ClearAnimation();

		// ── Interaction ──
		void SetInteractive(bool v)                    { m_interactive = v; }
		bool IsInteractive() const                     { return m_interactive; }
		UIInteractionState GetInteractionState() const { return m_interactionState; }
		bool IsHovered() const { return m_interactionState == UIInteractionState::Hovered; }
		bool IsPressed() const { return m_interactionState == UIInteractionState::Pressed; }
		bool IsHeld()     const { return m_interactionState == UIInteractionState::Held; }
		bool IsReleased() const { return m_interactionState == UIInteractionState::Released; }

		void SetOnClick(UICallback cb)      { m_onClick = std::move(cb); }
		void SetOnRelease(UICallback cb)    { m_onRelease = std::move(cb); }
		void SetOnHoverEnter(UICallback cb) { m_onHoverEnter = std::move(cb); }
		void SetOnHoverExit(UICallback cb)  { m_onHoverExit = std::move(cb); }
		void SetOnHeld(UICallback cb)       { m_onHeld = std::move(cb); }

		bool HitTest(const float2& screenPos) const;

		virtual void OnClick()      {}
		virtual void OnRelease()    {}
		virtual void OnHoverEnter() {}
		virtual void OnHoverExit()  {}
		virtual void OnHeld()       {}

		virtual void Initialize();
		virtual void SubmitUI(float dt = 0.f);

		virtual std::string GetTypeName() const { return "UIComponent"; }
		virtual MyJson      Serialize()   const;
		virtual void        Deserialize(const MyJson& j);

#ifdef _DEBUG
		virtual void OnInspectorGUI();
#endif

	protected:
		float4x4 GetOrthoViewProj() const;
		float    GetResolutionScale() const;
		float2   GetAnchorPoint() const;
		float4x4 GetUIWorldMatrix(float scale = 1.f) const;
		void     DoSubmitUI(Render::Geometry* geometry, Render::Material* material, float scale = 1.f);

		std::string m_name;
		UICanvas*   m_canvas   = nullptr;
		UIAnchor    m_anchor   = UIAnchor::TopLeft;
		float2      m_position = { 0.f, 0.f };
		float2      m_size     = { 100.f, 100.f };
		float       m_depth    = 0.f;
		float       m_rotation = 0.f;
		float2      m_pivot    = { 0.f, 0.f };  // 0~1 normalized, (0,0)=top-left for backward compat
		bool        m_dirty    = true;
		UIAnimation m_animation;

		// Interaction
		bool               m_interactive      = false;
		UIInteractionState m_interactionState = UIInteractionState::None;
		UICallback m_onClick;
		UICallback m_onRelease;
		UICallback m_onHoverEnter;
		UICallback m_onHoverExit;
		UICallback m_onHeld;

		Render::Geometry*                 m_quad     = nullptr;
		std::unique_ptr<Render::Material> m_material;
	};
}
