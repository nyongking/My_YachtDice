#pragma once
#include "RenderPassBase.h"
#include "RenderItem.h"
#include "ConstantBuffer.h"

namespace Render
{
	class GeometryPass : public RenderPassBase
	{
	public:
		explicit GeometryPass(Layer layer) : m_layer(layer) {}
		~GeometryPass() = default;

	public:
		bool Initialize(ID3D11Device* device) override;

		void Submit(const RenderCommand& cmd) override;

		void Execute(ID3D11DeviceContext* ctx) override;
		void Clear() override;

	private:
		void Sort();

		Layer                      m_layer;
		std::vector<RenderCommand> m_queue;

		// b0: PerFrame (viewProj) — 패스 시작 시 1회 업로드
		// b1: PerObject (world)   — 드로우콜마다 업로드
		ConstantBuffer m_cbPerFrame;
		ConstantBuffer m_cbPerObject;
	};
}
