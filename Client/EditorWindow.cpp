#include "ClientPch.h"
#include "EditorWindow.h"

#ifdef _DEBUG

#include "Scene.h"
#include "GameObject.h"
#include "CameraManager.h"
#include "CameraComponent.h"
#include "RigidBodyComponent.h"
#include "EngineGlobal.h"
#include "UIManager.h"
#include "UICanvas.h"
#include "UIComponent.h"

void EditorWindow::Render(GameEngine::Scene* scene)
{
	if (!scene) return;

	BuildHierarchyPanel(scene);
	BuildInspectorPanel();
	BuildUIPanel();
}

void EditorWindow::BuildHierarchyPanel(GameEngine::Scene* scene)
{
	ImGui::Begin("Hierarchy");

	ImGui::Checkbox("Draw Colliders", &GameEngine::GDebugDrawColliders);
	ImGui::Separator();

	for (auto& go : scene->GetGameObjects())
	{
		bool isCurrent = (go.get() == m_selectedGO);
		if (ImGui::Selectable(go->GetName().c_str(), isCurrent))
			m_selectedGO = go.get();
	}

	if (m_selectedGO)
	{
		bool found = false;
		for (auto& go : scene->GetGameObjects())
		{
			if (go.get() == m_selectedGO) { found = true; break; }
		}
		if (!found) m_selectedGO = nullptr;
	}

	ImGui::End();
}

void EditorWindow::BuildInspectorPanel()
{
	ImGui::Begin("Inspector");

	if (!m_selectedGO)
	{
		ImGui::TextDisabled("(No object selected)");
		ImGui::End();
		return;
	}

	ImGui::Text("%s", m_selectedGO->GetName().c_str());
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::RadioButton("Translate", m_gizmoOp == ImGuizmo::TRANSLATE)) m_gizmoOp = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate",    m_gizmoOp == ImGuizmo::ROTATE))    m_gizmoOp = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale",     m_gizmoOp == ImGuizmo::SCALE))     m_gizmoOp = ImGuizmo::SCALE;

		if (m_gizmoOp != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("World", m_gizmoMode == ImGuizmo::WORLD)) m_gizmoMode = ImGuizmo::WORLD;
			ImGui::SameLine();
			if (ImGui::RadioButton("Local", m_gizmoMode == ImGuizmo::LOCAL)) m_gizmoMode = ImGuizmo::LOCAL;
		}

		ImGui::Separator();

		auto* tr  = m_selectedGO->GetTransform();
		float3 pos = tr->GetPosition();
		float3 rot = tr->GetRotation();
		float3 scl = tr->GetScale();

		if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) tr->SetPosition(pos);
		if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f))  tr->SetRotation(rot);
		if (ImGui::DragFloat3("Scale",    &scl.x, 0.01f)) tr->SetScale(scl);
	}

	m_selectedGO->ForEachComponent([](GameEngine::Component* comp) {
		ImGui::PushID(comp);
		if (ImGui::CollapsingHeader(comp->GetTypeName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			comp->OnInspectorGUI();
		ImGui::PopID();
	});

	ImGui::End();

	BuildGizmo();
}

void EditorWindow::BuildUIPanel()
{
	auto& mgr = GameEngine::UIManager::GetInstance();

	ImGui::Begin("UI Editor");

	for (int i = 0; i < mgr.GetCanvasCount(); ++i)
	{
		auto* canvas = mgr.GetCanvas(i);
		ImGui::PushID(i);

		bool isGlobal = mgr.IsGlobal(i);
		const char* tag = isGlobal ? "[G]" : "[S]";

		bool canvasOpen = ImGui::TreeNodeEx(canvas->GetName().c_str(),
			(canvas == m_selectedCanvas) ? ImGuiTreeNodeFlags_Selected : 0,
			"%s %s", tag, canvas->GetName().c_str());

		if (ImGui::IsItemClicked())
		{
			m_selectedCanvas = canvas;
			m_selectedUIComp = nullptr;
		}

		if (canvasOpen)
		{
			bool visible = canvas->IsVisible();
			if (ImGui::Checkbox("Visible", &visible))
				canvas->SetVisible(visible);

			float depth = canvas->GetDepth();
			if (ImGui::DragFloat("Depth", &depth, 0.1f))
				canvas->SetDepth(depth);

			float2 pos = canvas->GetPosition();
			if (ImGui::DragFloat2("Position", &pos.x, 0.005f, 0.f, 1.f, "%.3f"))
				canvas->SetPosition(pos);

			float2 size = canvas->GetSize();
			if (ImGui::DragFloat2("Size", &size.x, 1.f))
				canvas->SetSize(size);

			if (canvas->GetPanel())
			{
				ImGui::Separator();
				ImGui::Text("Panel");
				float4 panelColor = canvas->GetPanel()->GetColor();
				if (ImGui::ColorEdit4("Panel Tint", &panelColor.x))
					canvas->GetPanel()->SetColor(panelColor);
			}

			ImGui::Separator();
			ImGui::Text("Components (%d)", canvas->GetComponentCount());

			for (int c = 0; c < canvas->GetComponentCount(); ++c)
			{
				auto* comp = canvas->GetComponent(c);
				ImGui::PushID(c);

				bool compSelected = (comp == m_selectedUIComp);
				char label[128];
				snprintf(label, sizeof(label), "[%d] %s (%s)",
					c, comp->GetName().c_str(), comp->GetTypeName().c_str());

				if (ImGui::Selectable(label, compSelected))
				{
					m_selectedUIComp = comp;
					m_selectedCanvas = canvas;
				}

				if (comp == m_selectedUIComp)
				{
					ImGui::Indent();
					comp->OnInspectorGUI();
					ImGui::Unindent();
				}

				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	if (mgr.GetCanvasCount() == 0)
		ImGui::TextDisabled("(No canvases)");

	ImGui::End();
}

void EditorWindow::BuildGizmo()
{
	auto* cam = GameEngine::CameraManager::GetInstance().GetMainCamera();
	if (!cam || !m_selectedGO)
		return;

	const auto* pView = cam->GetView();
	const auto* pProj = cam->GetProj();
	if (!pView || !pProj)
		return;

	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList(vp));
	ImGuizmo::SetRect(vp->Pos.x, vp->Pos.y, vp->Size.x, vp->Size.y);

	auto* tr = m_selectedGO->GetTransform();
	float4x4 worldMat = tr->GetWorldMatrix();
	ImGuizmo::MODE mode = (m_gizmoOp == ImGuizmo::SCALE) ? ImGuizmo::LOCAL : m_gizmoMode;

	if (ImGuizmo::Manipulate(
		reinterpret_cast<const float*>(pView),
		reinterpret_cast<const float*>(pProj),
		m_gizmoOp, mode,
		reinterpret_cast<float*>(&worldMat)))
	{
		float t[3], r[3], s[3];
		ImGuizmo::DecomposeMatrixToComponents(reinterpret_cast<float*>(&worldMat), t, r, s);
		tr->SetPosition({ t[0], t[1], t[2] });
		tr->SetRotation({ r[0], r[1], r[2] });
		tr->SetScale   ({ s[0], s[1], s[2] });
	}
}

#endif // _DEBUG
