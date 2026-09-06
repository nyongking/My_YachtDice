#pragma once

#ifdef _DEBUG

#include "imgui.h"
#include "ImGuizmo.h"

namespace GameEngine { class Scene; class GameObject; class UICanvas; class UIComponent; }

class EditorWindow
{
public:
	void Render(GameEngine::Scene* scene);

private:
	void BuildHierarchyPanel(GameEngine::Scene* scene);
	void BuildInspectorPanel();
	void BuildGizmo();
	void BuildUIPanel();

	GameEngine::GameObject*  m_selectedGO        = nullptr;
	GameEngine::UICanvas*    m_selectedCanvas     = nullptr;
	GameEngine::UIComponent* m_selectedUIComp     = nullptr;
	ImGuizmo::OPERATION      m_gizmoOp            = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE           m_gizmoMode          = ImGuizmo::WORLD;
};

#endif // _DEBUG
