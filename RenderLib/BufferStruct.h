#pragma once

namespace Render
{
	typedef struct POS
	{
		DirectX::XMFLOAT3		position;

		static const unsigned int			sNumElements = 1;
		static const D3D11_INPUT_ELEMENT_DESC sElements[sNumElements];
	}VTXPOS;

	typedef struct POSCOL
	{
		DirectX::XMFLOAT3		position;
		DirectX::XMFLOAT4		color;

		static const unsigned int			sNumElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC sElements[sNumElements];
	}VTXPOSCOL;

	typedef struct POSTEX
	{
		DirectX::XMFLOAT3		position;
		DirectX::XMFLOAT2		texcoord;

		static const unsigned int			sNumElements = 2;
		static const D3D11_INPUT_ELEMENT_DESC sElements[sNumElements];
	}VTXPOSTEX;

	// Static mesh vertex: Position, Normal, Tangent, UV
	typedef struct POSNORMTANUV
	{
		DirectX::XMFLOAT3		position;
		DirectX::XMFLOAT3		normal;
		DirectX::XMFLOAT3		tangent;
		DirectX::XMFLOAT2		texcoord;

		static const unsigned int			sNumElements = 4;
		static const D3D11_INPUT_ELEMENT_DESC sElements[sNumElements];
	}VTXPOSNORMTANUV;

	// Anim mesh vertex: Position, Normal, Tangent, UV, BlendIndex, BlendWeight
	typedef struct POSNORMTANUVBLEND
	{
		DirectX::XMFLOAT3		position;
		DirectX::XMFLOAT3		normal;
		DirectX::XMFLOAT3		tangent;
		DirectX::XMFLOAT2		texcoord;
		DirectX::XMUINT4		blendIndex;		// bone indices (up to 4)
		DirectX::XMFLOAT4		blendWeight;	// bone weights (sum = 1)

		static const unsigned int			sNumElements = 6;
		static const D3D11_INPUT_ELEMENT_DESC sElements[sNumElements];
	}VTXPOSNORMTANUVBLEND;
}