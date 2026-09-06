#include "GameEnginePch.h"
#include "Quad.h"
#include "BufferStruct.h"

using namespace DirectX;
using namespace Render;

bool GameEngine::Quad::DefaultCreateBuffers(ID3D11Device* pDevice)
{
	if (nullptr == pDevice)
		return false;

	m_vertexStride = sizeof(VTXPOSTEX);
	m_vertexCount = 4;
	m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region vertex
	std::vector<VTXPOSTEX> vertices;

	vertices.push_back(VTXPOSTEX{ XMFLOAT3(-0.5f, 0.5f, 0.f), XMFLOAT2(0.f, 0.f) });
	vertices.push_back(VTXPOSTEX{ XMFLOAT3(0.5f, 0.5f, 0.f), XMFLOAT2(1.f, 0.f) });
	vertices.push_back(VTXPOSTEX{ XMFLOAT3(0.5f, -0.5f, 0.f), XMFLOAT2(1.f, 1.f) });
	vertices.push_back(VTXPOSTEX{ XMFLOAT3(-0.5f, -0.5f, 0.f), XMFLOAT2(0.f, 1.f) });

	if (false == CreateVB(pDevice, vertices))
		return false;

#pragma endregion

#pragma region index
	std::vector<unsigned __int16> indices;

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	if (false == CreateIB<uint16_t>(pDevice, indices))
		return false;

#pragma endregion

	return true;
}
