#pragma once
#include "Geometry.h"
#include "BufferStruct.h"

namespace GameEngine
{
	class NineSliceGeometry : public Render::Geometry
	{
	public:
		NineSliceGeometry() = default;
		~NineSliceGeometry() = default;

		bool Create(ID3D11Device* device,
					float width, float height,
					float borderL, float borderT, float borderR, float borderB,
					float texWidth, float texHeight);

		bool UpdateVertices(ID3D11DeviceContext* context,
							float width, float height,
							float borderL, float borderT, float borderR, float borderB,
							float texWidth, float texHeight);

	private:
		void BuildVertices(std::vector<Render::VTXPOSTEX>& out,
						   float width, float height,
						   float borderL, float borderT, float borderR, float borderB,
						   float texWidth, float texHeight);
	};
}
