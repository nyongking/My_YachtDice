#include "GameEnginePch.h"
#include "DebugGeometry.h"
#include "BufferStruct.h"
#include <cmath>

using namespace DirectX;
using namespace Render;

namespace GameEngine
{
	// ── WireBox ─────────────────────────────────────
	// Unit cube (-0.5 ~ +0.5), 8 vertices, 24 indices (12 edges)

	bool WireBox::DefaultCreateBuffers(ID3D11Device* pDevice)
	{
		if (!pDevice) return false;

		const float h = 0.5f;
		XMFLOAT3 corners[8] =
		{
			{ -h, -h, -h }, {  h, -h, -h }, {  h,  h, -h }, { -h,  h, -h },
			{ -h, -h,  h }, {  h, -h,  h }, {  h,  h,  h }, { -h,  h,  h }
		};

		std::vector<VTXPOSCOL> vertices(8);
		for (int i = 0; i < 8; ++i)
			vertices[i] = { corners[i], m_color };

		std::vector<uint16_t> indices =
		{
			// bottom face edges
			0, 1,  1, 2,  2, 3,  3, 0,
			// top face edges
			4, 5,  5, 6,  6, 7,  7, 4,
			// vertical edges
			0, 4,  1, 5,  2, 6,  3, 7
		};

		return CreateBuffers(pDevice, vertices, indices,
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	// ── WireSphere ──────────────────────────────────
	// 3 circles (XY, XZ, YZ planes), radius 1.0, 24 segments each

	bool WireSphere::DefaultCreateBuffers(ID3D11Device* pDevice)
	{
		if (!pDevice) return false;

		constexpr int SEG = 24;
		constexpr float PI2 = 6.2831853f;

		std::vector<VTXPOSCOL> vertices;
		std::vector<uint16_t> indices;

		vertices.reserve(SEG * 3);
		indices.reserve(SEG * 2 * 3);

		auto addCircle = [&](int axis0, int axis1, int axisUp)
		{
			uint16_t base = static_cast<uint16_t>(vertices.size());
			for (int i = 0; i < SEG; ++i)
			{
				float angle = PI2 * i / SEG;
				float c = cosf(angle);
				float s = sinf(angle);

				XMFLOAT3 pos = { 0.f, 0.f, 0.f };
				reinterpret_cast<float*>(&pos)[axis0] = c;
				reinterpret_cast<float*>(&pos)[axis1] = s;

				vertices.push_back({ pos, m_color });

				uint16_t next = base + static_cast<uint16_t>((i + 1) % SEG);
				indices.push_back(base + static_cast<uint16_t>(i));
				indices.push_back(next);
			}
		};

		addCircle(0, 1, 2); // XY plane circle
		addCircle(0, 2, 1); // XZ plane circle
		addCircle(1, 2, 0); // YZ plane circle

		return CreateBuffers(pDevice, vertices, indices,
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	// ── WirePlane ───────────────────────────────────
	// XZ grid 10x10, each cell 1 unit, + normal arrow

	bool WirePlane::DefaultCreateBuffers(ID3D11Device* pDevice)
	{
		if (!pDevice) return false;

		std::vector<VTXPOSCOL> vertices;
		std::vector<uint16_t> indices;

		constexpr int HALF = 5;
		constexpr float EXTENT = static_cast<float>(HALF);

		// Grid lines along X axis (Z varies)
		for (int i = -HALF; i <= HALF; ++i)
		{
			uint16_t base = static_cast<uint16_t>(vertices.size());
			float z = static_cast<float>(i);
			vertices.push_back({ XMFLOAT3(-EXTENT, 0.f, z), m_color });
			vertices.push_back({ XMFLOAT3( EXTENT, 0.f, z), m_color });
			indices.push_back(base);
			indices.push_back(base + 1);
		}

		// Grid lines along Z axis (X varies)
		for (int i = -HALF; i <= HALF; ++i)
		{
			uint16_t base = static_cast<uint16_t>(vertices.size());
			float x = static_cast<float>(i);
			vertices.push_back({ XMFLOAT3(x, 0.f, -EXTENT), m_color });
			vertices.push_back({ XMFLOAT3(x, 0.f,  EXTENT), m_color });
			indices.push_back(base);
			indices.push_back(base + 1);
		}

		// Normal arrow (Y-up)
		{
			uint16_t base = static_cast<uint16_t>(vertices.size());
			vertices.push_back({ XMFLOAT3(0.f, 0.f, 0.f), m_color });
			vertices.push_back({ XMFLOAT3(0.f, 1.f, 0.f), m_color });
			indices.push_back(base);
			indices.push_back(base + 1);
		}

		return CreateBuffers(pDevice, vertices, indices,
			D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
}
