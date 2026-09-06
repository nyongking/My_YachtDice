#pragma once
#include "Ray.h"

namespace GameEngine
{
	class CameraComponent;

	class CameraManager
	{
	public:
		static CameraManager& GetInstance()
		{
			static CameraManager instance;
			return instance;
		}

		void             SetMainCamera(CameraComponent* cam) { m_mainCamera = cam; }
		CameraComponent* GetMainCamera() const               { return m_mainCamera; }

		// 마우스 위치 → 메인 카메라 기준 월드 Ray (layerMask 지정 가능)
		Ray ScreenToWorldRay(unsigned int layerMask = 0xFFFFFFFF) const;

		// 월드 좌표 → 스크린 픽셀 좌표 (UI 배치용)
		// 반환: (pixelX, pixelY), 카메라 뒤면 (-1, -1)
		float2 WorldToScreen(const float3& worldPos) const;

	private:
		CameraManager() = default;
		CameraManager(const CameraManager&) = delete;
		CameraManager& operator=(const CameraManager&) = delete;

		CameraComponent* m_mainCamera = nullptr;
	};
}
