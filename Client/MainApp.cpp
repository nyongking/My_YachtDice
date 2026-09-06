#include "ClientPch.h"
#include "MainApp.h"

#include "SceneManager.h"
#include "GameScene.h"

#include "Renderer.h"
#include "RenderDevice.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

#include "ClientSession.h"
#include "Service.h"
#include "SocketUtils.h"
#include "IocpCore.h"


MainApp::MainApp()
{
}

MainApp::~MainApp()
{
	// Clear thread-local SendBufferChunk before service cleanup
	// to prevent PushGlobal from re-entering Lock during destruction
	LrefSendBufferChunk = nullptr;

	if (m_clientService)
	{
		m_clientService->CloseService();
		m_clientService.reset();
	}

	ImGuiShutdown();
	m_engine.Shutdown();
	ReleaseGame();
	SocketUtils::Clear();
	ReleaseServerCore();
	ReleaseCore();
}

bool MainApp::Init()
{
	InitGame();
	InitNetwork();

	GameEngine::SceneManager::GetInstance().LoadScene<GameScene>();

	GameEngine::EngineCallbacks cb;
	cb.onBeginFrame = [this]() { ImGuiBeginFrame(); };
	cb.onEndFrame   = [this]() { ImGuiEndFrame(); };

	GameEngine::EngineConfig config;
	config.targetFPS = 144;

	m_engine.Init(config, cb);

	ImGuiInit();

	return true;
}

void MainApp::Loop()
{
	NetworkTick();
	m_engine.Tick();

#ifdef _DEBUG
	auto& timer = m_engine.GetTimer();
	wsprintf(m_szBuf, TEXT("FPS: %d / dt: %.4f"), timer.GetFPS(), timer.GetDeltaTime());
	SetWindowText(wnd, m_szBuf);
#endif
}

void MainApp::InitNetwork()
{
	m_clientService = std::make_shared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		std::make_shared<IocpCore>(),
		[]() -> RefSession
		{
			auto session = std::make_shared<ClientSession>();
			GClientSession = session;
			return session;
		},
		1
	);

	m_clientService->Start();
}

void MainApp::NetworkTick()
{
	if (m_clientService)
	{
		m_clientService->GetIocpCore()->Dispatch(0);
	}
}

void MainApp::ImGuiInit()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(wnd);
	ImGui_ImplDX11_Init(
		Render::RenderDevice::GetInstance().GetDevice().Get(),
		Render::RenderDevice::GetInstance().GetContext().Get());
}

void MainApp::ImGuiShutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void MainApp::ImGuiBeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void MainApp::ImGuiEndFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}
