#pragma once

#include "RenderTypes.h"
#include "Vec3.h"
#include "Quat.h"
#include <DirectXMath.h>

namespace GameEngine
{
	// ── Vec3 ↔ float3 ──────────────────────────────

	inline Vec3   ToPhysics(const float3& v) { return Vec3(v.x, v.y, v.z); }
	inline float3 ToEngine(const Vec3& v)    { return float3{ v.x, v.y, v.z }; }

	// ── Euler(degrees) → Physics Quat ──────────────
	// Transform::GetWorldMatrix()와 동일한 회전 순서: Rx * Ry * Rz (row-major)

	inline Quat EulerToPhysicsQuat(const float3& eulerDeg)
	{
		using namespace DirectX;
		const float toRad = XM_PI / 180.f;

		XMMATRIX R = XMMatrixRotationX(eulerDeg.x * toRad)
		           * XMMatrixRotationY(eulerDeg.y * toRad)
		           * XMMatrixRotationZ(eulerDeg.z * toRad);

		XMVECTOR qv = XMQuaternionRotationMatrix(R);
		XMFLOAT4 qf;
		XMStoreFloat4(&qf, qv);

		return Quat(qf.x, qf.y, qf.z, qf.w);
	}

	// ── Physics Quat → Euler(degrees) ──────────────
	// Rx * Ry * Rz (intrinsic XYZ) 순서로 분해

	inline float3 PhysicsQuatToEuler(const Quat& q)
	{
		using namespace DirectX;
		const float toDeg = 180.f / XM_PI;

		XMVECTOR qv = XMVectorSet(q.x, q.y, q.z, q.w);
		XMMATRIX R   = XMMatrixRotationQuaternion(qv);

		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, R);

		// Rx*Ry*Rz 행렬에서:
		//   m[0][2] = -sin(yaw)
		//   m[1][2] =  sin(pitch)*cos(yaw)
		//   m[2][2] =  cos(pitch)*cos(yaw)
		//   m[0][1] =  cos(yaw)*sin(roll)
		//   m[0][0] =  cos(yaw)*cos(roll)

		float pitch, yaw, roll;
		float sinY = -m.m[0][2];

		if (fabsf(sinY) < 0.99999f)
		{
			yaw   = asinf(sinY);
			pitch = atan2f(m.m[1][2], m.m[2][2]);
			roll  = atan2f(m.m[0][1], m.m[0][0]);
		}
		else
		{
			// 짐벌 락
			yaw   = sinY > 0.f ? XM_PIDIV2 : -XM_PIDIV2;
			pitch = atan2f(-m.m[2][1], m.m[1][1]);
			roll  = 0.f;
		}

		return float3{ pitch * toDeg, yaw * toDeg, roll * toDeg };
	}
}
