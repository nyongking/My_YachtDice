#pragma once
#include "UICanvas.h"
#include <vector>
#include <memory>
#include <string>

namespace GameEngine
{

	class UIManager
	{
	public:
		static UIManager& GetInstance()
		{
			static UIManager instance;
			return instance;
		}

		UICanvas* CreateCanvas(const std::string& name, bool global = false);
		void      RemoveCanvas(const std::string& name);
		UICanvas* FindCanvas(const std::string& name) const;

		void SetCanvasVisible(const std::string& name, bool v);
		void SetCanvasPosition(const std::string& name, const float2& normalized);
		void SetCanvasPositionScreen(const std::string& name, const float2& screenPos);

		void ClearSceneCanvases();
		void ClearAll();

		void SubmitUI(float dt = 0.f);
		void ProcessInteraction();
		void NotifyComponentRemoved(UIComponent* comp);

		void LoadSingleCanvasFromJson(const MyJson& j, bool global = false);
		void LoadCanvasesFromJson(const MyJson& arr, bool global = false);

		int GetCanvasCount() const { return static_cast<int>(m_canvases.size()); }
		UICanvas* GetCanvas(int index) const { return m_canvases[index].canvas.get(); }
		bool IsGlobal(int index) const { return m_canvases[index].global; }

	private:
		UIManager() = default;
		UIManager(const UIManager&) = delete;
		UIManager& operator=(const UIManager&) = delete;

		void SortByDepth();
		UIComponent* HitTestAll(const float2& mousePos) const;

		struct CanvasEntry
		{
			std::unique_ptr<UICanvas> canvas;
			bool global = false;
		};

		std::vector<CanvasEntry> m_canvases;
		bool m_dirty = true;

		UIComponent* m_hoveredComponent = nullptr;
		UIComponent* m_pressedComponent = nullptr;
	};
}
