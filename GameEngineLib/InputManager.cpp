#include "GameEnginePch.h"
#include "InputManager.h"

namespace GameEngine
{
	void InputManager::Initialize(HWND hwnd)
	{
		m_keyboard = std::make_unique<DirectX::Keyboard>();
		m_mouse    = std::make_unique<DirectX::Mouse>();
		m_mouse->SetWindow(hwnd);
		m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	}

	void InputManager::Update()
	{
		// Keyboard
		m_keyState = m_keyboard->GetState();
		m_keyTracker.Update(m_keyState);

		// Mouse
		m_mouseState = m_mouse->GetState();
		m_mouseTracker.Update(m_mouseState);

		float2 curPos{ static_cast<float>(m_mouseState.x),
		               static_cast<float>(m_mouseState.y) };

		m_mouseDelta.x = curPos.x - m_prevMousePos.x;
		m_mouseDelta.y = curPos.y - m_prevMousePos.y;
		m_prevMousePos = curPos;

		m_scrollDelta = m_mouseState.scrollWheelValue - m_prevScrollValue;
		m_prevScrollValue = m_mouseState.scrollWheelValue;
	}

	void InputManager::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		DirectX::Keyboard::ProcessMessage(msg, wParam, lParam);
		DirectX::Mouse::ProcessMessage(msg, wParam, lParam);
	}

	// ── Keyboard ──
	bool InputManager::IsKeyDown(DirectX::Keyboard::Keys key) const
	{
		return m_keyState.IsKeyDown(key);
	}

	bool InputManager::IsKeyPressed(DirectX::Keyboard::Keys key) const
	{
		return m_keyTracker.IsKeyPressed(key);
	}

	bool InputManager::IsKeyReleased(DirectX::Keyboard::Keys key) const
	{
		return m_keyTracker.IsKeyReleased(key);
	}

	// ── Mouse ──
	bool InputManager::IsMouseButtonDown(int button) const
	{
		switch (button)
		{
		case 0: return m_mouseState.leftButton;
		case 1: return m_mouseState.rightButton;
		case 2: return m_mouseState.middleButton;
		default: return false;
		}
	}

	bool InputManager::IsMouseButtonPressed(int button) const
	{
		using BState = DirectX::Mouse::ButtonStateTracker::ButtonState;
		switch (button)
		{
		case 0: return m_mouseTracker.leftButton  == BState::PRESSED;
		case 1: return m_mouseTracker.rightButton  == BState::PRESSED;
		case 2: return m_mouseTracker.middleButton == BState::PRESSED;
		default: return false;
		}
	}

	bool InputManager::IsMouseButtonReleased(int button) const
	{
		using BState = DirectX::Mouse::ButtonStateTracker::ButtonState;
		switch (button)
		{
		case 0: return m_mouseTracker.leftButton  == BState::RELEASED;
		case 1: return m_mouseTracker.rightButton  == BState::RELEASED;
		case 2: return m_mouseTracker.middleButton == BState::RELEASED;
		default: return false;
		}
	}

	float2 InputManager::GetMousePosition() const
	{
		return { static_cast<float>(m_mouseState.x),
		         static_cast<float>(m_mouseState.y) };
	}

	float2 InputManager::GetMouseDelta() const
	{
		return m_mouseDelta;
	}

	int InputManager::GetScrollWheelDelta() const
	{
		return m_scrollDelta;
	}

	void InputManager::SetMouseMode(DirectX::Mouse::Mode mode)
	{
		m_mouse->SetMode(mode);
	}

	void InputManager::SetMouseVisible(bool visible)
	{
		m_mouse->SetVisible(visible);
	}
}
