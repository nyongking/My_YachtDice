#include "GameEnginePch.h"
#include "UIComponent.h"
#include "UICanvas.h"
#include "RenderItem.h"
#include "RenderPipeline.h"
#include "Renderer.h"
#include "GeometryManager.h"
#include "MaterialManager.h"
#include "Material.h"
#include "Geometry.h"

#ifdef _DEBUG
#include "Imgui/imgui.h"
#endif

using namespace DirectX;

namespace GameEngine
{
	UIComponent::~UIComponent() = default;

	void UIComponent::Initialize()
	{
		m_quad = GeometryManager::GetInstance()->Get("Quad");
		m_material = MaterialManager::GetInstance()->Get("UIMaterial");
	}

	void UIComponent::SetAnimation(UIAnimation::Type type, float amplitude, float speed)
	{
		m_animation.type      = type;
		m_animation.amplitude = amplitude;
		m_animation.speed     = speed;
		m_animation.Reset();
	}

	void UIComponent::ClearAnimation()
	{
		m_animation.type = UIAnimation::Type::None;
		m_animation.Reset();
	}

	bool UIComponent::HitTest(const float2& screenPos) const
	{
		if (!m_interactive) return false;

		float resolutionScale = GetResolutionScale();
		float2 anchorPt = GetAnchorPoint();
		float pivotX = anchorPt.x + m_position.x * resolutionScale;
		float pivotY = anchorPt.y + m_position.y * resolutionScale;

		float width = m_size.x * resolutionScale;
		float height = m_size.y * resolutionScale;
		float ltX = pivotX - width * m_pivot.x;
		float ltY = pivotY - height * m_pivot.y;

		return screenPos.x >= ltX && screenPos.x <= ltX + width
			&& screenPos.y >= ltY && screenPos.y <= ltY + height;
	}

	void UIComponent::SubmitUI(float dt)
	{
		if (!m_quad || !m_material)
			return;
		float scale = m_animation.Update(dt);
		DoSubmitUI(m_quad, m_material.get(), scale);
	}

	float4x4 UIComponent::GetOrthoViewProj() const
	{
		float w = static_cast<float>(Render::Renderer::GetInstance().GetWidth());
		float h = static_cast<float>(Render::Renderer::GetInstance().GetHeight());

		XMMATRIX ortho = XMMatrixOrthographicOffCenterLH(0.f, w, 0.f, h, 0.f, 1000.f);

		float4x4 result;
		XMStoreFloat4x4(&result, ortho);
		return result;
	}
	float UIComponent::GetResolutionScale() const
	{
		return m_canvas ? m_canvas->GetResolutionScale() : 1.f;
	}

	float2 UIComponent::GetAnchorPoint() const
	{
		// anchor determines which point on the canvas this component attaches to
		float2 canvasPos = { 0.f, 0.f };
		float2 canvasSize = { 0.f, 0.f };

		if (m_canvas)
		{
			canvasPos  = m_canvas->GetScreenPosition();
			canvasSize = m_canvas->GetScaledSize();
		}

		// canvas LTRB
		float lt_x = canvasPos.x - canvasSize.x * 0.5f;
		float lt_y = canvasPos.y - canvasSize.y * 0.5f;
		float rb_x = canvasPos.x + canvasSize.x * 0.5f;
		float rb_y = canvasPos.y + canvasSize.y * 0.5f;
		float cx   = canvasPos.x;
		float cy   = canvasPos.y;

		switch (m_anchor)
		{
		case UIAnchor::TopLeft:     return { lt_x, lt_y };
		case UIAnchor::Top:         return { cx,   lt_y };
		case UIAnchor::TopRight:    return { rb_x, lt_y };
		case UIAnchor::Left:        return { lt_x, cy   };
		case UIAnchor::Center:      return { cx,   cy   };
		case UIAnchor::Right:       return { rb_x, cy   };
		case UIAnchor::BottomLeft:  return { lt_x, rb_y };
		case UIAnchor::Bottom:      return { cx,   rb_y };
		case UIAnchor::BottomRight: return { rb_x, rb_y };
		default:                    return { lt_x, lt_y };
		}
	}

	float4x4 UIComponent::GetUIWorldMatrix(float animScale) const
	{
		float screenH = static_cast<float>(Render::Renderer::GetInstance().GetHeight());

		float resolutionScale = GetResolutionScale();
		float2 anchorPt = GetAnchorPoint();

		// position points to where the pivot of this element should be
		float pivotScreenX = anchorPt.x + m_position.x * resolutionScale;
		float pivotScreenY = anchorPt.y + m_position.y * resolutionScale;

		float sw = m_size.x * animScale * resolutionScale;
		float sh = m_size.y * animScale * resolutionScale;

		// top-left derived from pivot
		float ltX = pivotScreenX - sw * m_pivot.x;
		float ltY = pivotScreenY - sh * m_pivot.y;

		float centerX = ltX + sw * 0.5f;
		float centerY = ltY + sh * 0.5f;

		float renderX = centerX;
		float renderY = screenH - centerY;

		XMMATRIX scale = XMMatrixScaling(sw, sh, 1.f);
		XMMATRIX rot   = XMMatrixRotationZ(m_rotation);
		XMMATRIX trans = XMMatrixTranslation(renderX, renderY, m_depth);
		XMMATRIX world = scale * rot * trans;

		float4x4 result;
		XMStoreFloat4x4(&result, world);
		return result;
	}

	void UIComponent::DoSubmitUI(Render::Geometry* geometry, Render::Material* material, float scale)
	{
		if (!geometry || !material)
			return;

		Render::RenderCommand cmd;
		cmd.geometry = geometry;
		cmd.material = material;
		cmd.world    = GetUIWorldMatrix(scale);
		cmd.viewProj = GetOrthoViewProj();

		Render::RenderPipeline::GetInstance().Submit(
			Render::RenderPassBase::Layer::UI, cmd);
	}

	static std::string AnchorToString(UIAnchor a)
	{
		switch (a)
		{
		case UIAnchor::TopLeft:     return "TopLeft";
		case UIAnchor::Top:         return "Top";
		case UIAnchor::TopRight:    return "TopRight";
		case UIAnchor::Left:        return "Left";
		case UIAnchor::Center:      return "Center";
		case UIAnchor::Right:       return "Right";
		case UIAnchor::BottomLeft:  return "BottomLeft";
		case UIAnchor::Bottom:      return "Bottom";
		case UIAnchor::BottomRight: return "BottomRight";
		default:                    return "TopLeft";
		}
	}

	static UIAnchor AnchorFromString(const std::string& s)
	{
		if (s == "Top")         return UIAnchor::Top;
		if (s == "TopRight")    return UIAnchor::TopRight;
		if (s == "Left")        return UIAnchor::Left;
		if (s == "Center")      return UIAnchor::Center;
		if (s == "Right")       return UIAnchor::Right;
		if (s == "BottomLeft")  return UIAnchor::BottomLeft;
		if (s == "Bottom")      return UIAnchor::Bottom;
		if (s == "BottomRight") return UIAnchor::BottomRight;
		return UIAnchor::TopLeft;
	}

	MyJson UIComponent::Serialize() const
	{
		MyJson j;
		j["type"]     = GetTypeName();
		if (!m_name.empty())
			j["name"] = m_name;
		j["anchor"]   = AnchorToString(m_anchor);
		j["posX"]     = m_position.x;
		j["posY"]     = m_position.y;
		j["width"]    = m_size.x;
		j["height"]   = m_size.y;
		j["depth"]    = m_depth;
		if (m_rotation != 0.f)
			j["rotation"] = XMConvertToDegrees(m_rotation);
		if (m_pivot.x != 0.f || m_pivot.y != 0.f)
		{
			j["pivotX"] = m_pivot.x;
			j["pivotY"] = m_pivot.y;
		}
		if (m_interactive)
			j["interactive"] = true;
		if (m_animation.type != UIAnimation::Type::None)
			j["animation"] = m_animation.Serialize();
		return j;
	}

	void UIComponent::Deserialize(const MyJson& j)
	{
		if (j.contains("name"))
			m_name = j["name"].get<std::string>();
		if (j.contains("anchor"))
			m_anchor = AnchorFromString(j["anchor"].get<std::string>());
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
		if (j.contains("rotation"))
			m_rotation = XMConvertToRadians(j["rotation"].get<float>());
		if (j.contains("pivotX"))
			m_pivot.x = j["pivotX"].get<float>();
		if (j.contains("pivotY"))
			m_pivot.y = j["pivotY"].get<float>();
		if (j.contains("interactive"))
			m_interactive = j["interactive"].get<bool>();
		if (j.contains("animation"))
			m_animation.Deserialize(j["animation"]);

		m_dirty = true;
	}

#ifdef _DEBUG
	void UIComponent::OnInspectorGUI()
	{
		const char* anchorNames[] = {
			"TopLeft", "Top", "TopRight",
			"Left", "Center", "Right",
			"BottomLeft", "Bottom", "BottomRight"
		};
		int anchorInt = static_cast<int>(m_anchor);
		if (ImGui::Combo("Anchor", &anchorInt, anchorNames, 9))
			m_anchor = static_cast<UIAnchor>(anchorInt);

		ImGui::DragFloat2("Position", &m_position.x, 1.f);
		ImGui::DragFloat2("Pivot", &m_pivot.x, 0.01f, 0.f, 1.f);

		if (ImGui::DragFloat2("Size", &m_size.x, 1.f, 1.f, 4096.f))
			m_dirty = true;

		ImGui::DragFloat("Depth", &m_depth, 0.1f, 0.f, 999.f);

		float rotDeg = XMConvertToDegrees(m_rotation);
		if (ImGui::DragFloat("Rotation", &rotDeg, 1.f, -360.f, 360.f, "%.1f deg"))
			m_rotation = XMConvertToRadians(rotDeg);

		ImGui::Checkbox("Interactive", &m_interactive);
		if (m_interactive)
		{
			const char* stateNames[] = { "None", "Hovered", "Pressed", "Held", "Released" };
			ImGui::Text("State: %s", stateNames[static_cast<int>(m_interactionState)]);
		}
	}
#endif
}
