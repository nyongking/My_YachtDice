#include "GameEnginePch.h"
#include "CameraManager.h"
#include "CameraComponent.h"
#include "InputManager.h"
#include "Renderer.h"

namespace GameEngine
{
	Ray CameraManager::ScreenToWorldRay(unsigned int layerMask) const
	{
		Ray ray{};
		if (!m_mainCamera)
			return ray;

		float2 mousePos = InputManager::GetInstance().GetMousePosition();
		ray = m_mainCamera->ScreenToWorldRay(mousePos.x, mousePos.y);
		ray.layerMask = layerMask;
		return ray;
	}

	float2 CameraManager::WorldToScreen(const float3& worldPos) const
	{
		using namespace DirectX;

		if (!m_mainCamera || !m_mainCamera->GetViewProj())
			return { -1.f, -1.f };

		XMMATRIX vp = XMLoadFloat4x4(m_mainCamera->GetViewProj());
		XMVECTOR pos = XMVectorSet(worldPos.x, worldPos.y, worldPos.z, 1.f);
		XMVECTOR clip = XMVector4Transform(pos, vp);

		float w = XMVectorGetW(clip);
		if (w <= 0.f)
			return { -1.f, -1.f };

		float ndcX = XMVectorGetX(clip) / w;
		float ndcY = XMVectorGetY(clip) / w;

		float screenW = static_cast<float>(Render::Renderer::GetInstance().GetWidth());
		float screenH = static_cast<float>(Render::Renderer::GetInstance().GetHeight());

		float pixelX = (ndcX + 1.f) * 0.5f * screenW;
		float pixelY = (1.f - ndcY) * 0.5f * screenH;  // Y 반전

		return { pixelX, pixelY };
	}
}
