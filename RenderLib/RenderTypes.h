#pragma once

using float4x4 = DirectX::XMFLOAT4X4;
using _matrix = DirectX::XMMATRIX;
using fmatrix = DirectX::FXMMATRIX;
using cmatrix = DirectX::CXMMATRIX;

using uint2 = DirectX::XMUINT2;
using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

using _vector = DirectX::XMVECTOR;
using fvector = DirectX::FXMVECTOR;
using gvector = DirectX::GXMVECTOR;
using hvector = DirectX::HXMVECTOR;
using cvector = DirectX::CXMVECTOR;

template<typename T>
using RefCom = Microsoft::WRL::ComPtr<T>;