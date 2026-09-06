#include "RenderPch.h"
#include "ViewProjMatrix.h"

namespace Render
{
	void ViewProjMatrix::UpdateView(fmatrix view)
	{
		DirectX::XMStoreFloat4x4(&m_view, view);
		DirectX::XMStoreFloat4x4(&m_inverseview, DirectX::XMMatrixInverse(nullptr, view));
		DirectX::XMStoreFloat4x4(&m_viewproj, DirectX::XMMatrixMultiply(view, DirectX::XMLoadFloat4x4(&m_proj)));
	}

	void ViewProjMatrix::UpdateProj(fmatrix proj)
	{
		DirectX::XMStoreFloat4x4(&m_proj, proj);
		DirectX::XMStoreFloat4x4(&m_viewproj, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&m_view), proj));
	}

	void ViewProjMatrix::UpdateViewProj(fmatrix view, cmatrix proj)
	{
		DirectX::XMStoreFloat4x4(&m_view, view);
		DirectX::XMStoreFloat4x4(&m_proj, proj);
		DirectX::XMStoreFloat4x4(&m_inverseview, DirectX::XMMatrixInverse(nullptr, view));
		DirectX::XMStoreFloat4x4(&m_viewproj, DirectX::XMMatrixMultiply(view, proj));
	}

}

