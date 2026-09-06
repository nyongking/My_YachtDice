#include "RenderPch.h"
#include "Geometry.h"

namespace Render
{
	bool Geometry::BindAndDraw(ID3D11DeviceContext* pContext)
	{
		if (nullptr == pContext || nullptr == m_vb || nullptr == m_ib)
			return false;

		UINT32 offset = 0;
		pContext->IASetVertexBuffers(0, 1, m_vb.GetAddressOf(), &m_vertexStride, &offset);
		pContext->IASetPrimitiveTopology(m_topology);
		pContext->IASetIndexBuffer(m_ib.Get(), m_indexFormat, 0);

		// draw
		pContext->DrawIndexed(m_indexCount, 0, 0);

		return true;
	}


}

