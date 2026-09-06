#pragma once
#include "Engine.h"
#include "EditorWindow.h"

class MainApp
{
public:
	MainApp();
	~MainApp();

public:
	bool Init();
	void Loop();

private:
	void InitNetwork();
	void NetworkTick();

	void ImGuiInit();
	void ImGuiShutdown();
	void ImGuiBeginFrame();
	void ImGuiEndFrame();

	GameEngine::Engine m_engine;
	RefClientService   m_clientService;

#ifdef _DEBUG
	EditorWindow       m_editor;
	TCHAR m_szBuf[64];
#endif
};
