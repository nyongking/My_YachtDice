#pragma once

namespace Render
{
	class ViewProjMatrix
	{
	public:
		ViewProjMatrix() = default;
		~ViewProjMatrix() = default;

	public:
		void UpdateView(fmatrix view);
		void UpdateProj(fmatrix proj);
		void UpdateViewProj(fmatrix view, cmatrix proj);

		const float4x4* GetView() const { return &m_view; }
		const float4x4* GetProj() const { return &m_proj; }
		const float4x4* GetViewProj() const { return &m_viewproj; }
		const float4x4* GetInverseView() const { return &m_inverseview; }

	private:
		float4x4 m_view;
		float4x4 m_proj;
		float4x4 m_viewproj;
		float4x4 m_inverseview;
	};

}