#include "GameEnginePch.h"
#include "NineSliceGeometry.h"

using namespace DirectX;
using namespace Render;

namespace GameEngine
{
	void NineSliceGeometry::BuildVertices(std::vector<VTXPOSTEX>& out,
										  float width, float height,
										  float borderL, float borderT, float borderR, float borderB,
										  float texWidth, float texHeight)
	{
		float hw = width  * 0.5f;
		float hh = height * 0.5f;

		float px[4] = { -hw, -hw + borderL, hw - borderR, hw };
		float py[4] = { -hh, -hh + borderT, hh - borderB, hh };

		float uL = (texWidth  > 0.f) ? borderL / texWidth  : 0.f;
		float uR = (texWidth  > 0.f) ? 1.f - borderR / texWidth  : 1.f;
		float vT = (texHeight > 0.f) ? borderT / texHeight : 0.f;
		float vB = (texHeight > 0.f) ? 1.f - borderB / texHeight : 1.f;

		float u[4] = { 0.f, uL, uR, 1.f };
		float v[4] = { 0.f, vT, vB, 1.f };

		out.resize(16);
		for (int row = 0; row < 4; ++row)
		{
			for (int col = 0; col < 4; ++col)
			{
				int idx = row * 4 + col;
				out[idx].position = XMFLOAT3(px[col], py[row], 0.f);
				out[idx].texcoord = XMFLOAT2(u[col], v[row]);
			}
		}
	}

	bool NineSliceGeometry::Create(ID3D11Device* device,
								   float width, float height,
								   float borderL, float borderT, float borderR, float borderB,
								   float texWidth, float texHeight)
	{
		if (!device)
			return false;

		m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		std::vector<VTXPOSTEX> vertices;
		BuildVertices(vertices, width, height, borderL, borderT, borderR, borderB, texWidth, texHeight);

		if (!CreateDynamicVB(device, vertices))
			return false;

		std::vector<uint16_t> indices;
		indices.reserve(54);

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				uint16_t tl = static_cast<uint16_t>(row * 4 + col);
				uint16_t tr = tl + 1;
				uint16_t bl = tl + 4;
				uint16_t br = bl + 1;

				indices.push_back(tl);
				indices.push_back(tr);
				indices.push_back(br);

				indices.push_back(tl);
				indices.push_back(br);
				indices.push_back(bl);
			}
		}

		if (!CreateIB(device, indices))
			return false;

		return true;
	}

	bool NineSliceGeometry::UpdateVertices(ID3D11DeviceContext* context,
										   float width, float height,
										   float borderL, float borderT, float borderR, float borderB,
										   float texWidth, float texHeight)
	{
		std::vector<VTXPOSTEX> vertices;
		BuildVertices(vertices, width, height, borderL, borderT, borderR, borderB, texWidth, texHeight);

		return UpdateVB(context, vertices.data(),
						static_cast<unsigned int>(vertices.size() * sizeof(VTXPOSTEX)));
	}
}
