#pragma once

#include <DirectXTK/Keyboard.h>
#include <DirectXTK/Mouse.h>
#include "RenderTypes.h"

namespace GameEngine
{
	class InputManager
	{
	public:
		static InputManager& GetInstance()
		{
			static InputManager instance;
			return instance;
		}

		void Initialize(HWND hwnd);
		void Update();
		void ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);

		// ── Keyboard ──
		bool IsKeyDown(DirectX::Keyboard::Keys key) const;
		bool IsKeyPressed(DirectX::Keyboard::Keys key) const;
		bool IsKeyReleased(DirectX::Keyboard::Keys key) const;

		// ── Mouse ──
		bool IsMouseButtonDown(int button) const;      // 0=Left, 1=Right, 2=Middle
		bool IsMouseButtonPressed(int button) const;
		bool IsMouseButtonReleased(int button) const;

		float2 GetMousePosition() const;
		float2 GetMouseDelta() const;
		int    GetScrollWheelDelta() const;

		void   SetMouseMode(DirectX::Mouse::Mode mode);
		void   SetMouseVisible(bool visible);

	private:
		InputManager() = default;
		InputManager(const InputManager&) = delete;
		InputManager& operator=(const InputManager&) = delete;

		std::unique_ptr<DirectX::Keyboard> m_keyboard;
		std::unique_ptr<DirectX::Mouse>    m_mouse;

		DirectX::Keyboard::KeyboardStateTracker m_keyTracker;
		DirectX::Mouse::ButtonStateTracker      m_mouseTracker;

		DirectX::Keyboard::State m_keyState{};
		DirectX::Mouse::State    m_mouseState{};

		float2 m_prevMousePos{ 0.f, 0.f };
		float2 m_mouseDelta{ 0.f, 0.f };
		int    m_scrollDelta = 0;
		int    m_prevScrollValue = 0;
	};
}
