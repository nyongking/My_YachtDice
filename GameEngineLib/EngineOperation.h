#pragma once
#include "DirectXMath.h"

#define RANDOM__MAX 0x7fff

inline float RandFloat(float lo, float hi)
{
	return lo + static_cast<float>(rand()) / RANDOM__MAX * (hi - lo);
}

inline int RandInt(int lo, int hi)
{
	return lo + static_cast<int>(rand()) % (hi - lo + 1);
}

inline DirectX::XMVECTOR XM_CALLCONV UpVector()
{
	return DirectX::g_XMIdentityR1;
}

inline DirectX::XMVECTOR XM_CALLCONV LookVector()
{
	return DirectX::g_XMIdentityR2;
}

inline DirectX::XMVECTOR XM_CALLCONV LeftVector()
{
	return DirectX::g_XMNegIdentityR0;
}

inline DirectX::XMVECTOR XM_CALLCONV RightVector()
{
	return DirectX::g_XMIdentityR0;
}

inline DirectX::XMFLOAT3 Lerp(const DirectX::XMFLOAT3& v1, const DirectX::XMFLOAT3& v2, float t)
{
	DirectX::XMFLOAT3 ret;

	XMStoreFloat3(&ret, DirectX::XMVectorLerp(
		DirectX::XMLoadFloat3(&v1),
		DirectX::XMLoadFloat3(&v2),
		t));	

	return ret;
}