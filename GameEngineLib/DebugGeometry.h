#pragma once
#include "Geometry.h"

namespace GameEngine
{
	class WireBox : public Render::Geometry
	{
	public:
		explicit WireBox(const DirectX::XMFLOAT4& color) : m_color(color) {}
		bool DefaultCreateBuffers(ID3D11Device* pDevice) override;
	private:
		DirectX::XMFLOAT4 m_color;
	};

	class WireSphere : public Render::Geometry
	{
	public:
		explicit WireSphere(const DirectX::XMFLOAT4& color) : m_color(color) {}
		bool DefaultCreateBuffers(ID3D11Device* pDevice) override;
	private:
		DirectX::XMFLOAT4 m_color;
	};

	class WirePlane : public Render::Geometry
	{
	public:
		explicit WirePlane(const DirectX::XMFLOAT4& color) : m_color(color) {}
		bool DefaultCreateBuffers(ID3D11Device* pDevice) override;
	private:
		DirectX::XMFLOAT4 m_color;
	};
}
