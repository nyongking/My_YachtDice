#pragma once
#include "RenderPassBase.h"
#include "RenderItem.h"
#include "TextItem.h"
#include "ConstantBuffer.h"
#include "DirectXTK/SpriteBatch.h"
#include <memory>

namespace Render
{
	class UIPass : public RenderPassBase
	{
	public:
		UIPass() = default;
		~UIPass();

	public:
		bool Initialize(ID3D11Device* device) override;

		void Submit(const RenderCommand& cmd) override;
		void SubmitText(const TextCommand& cmd);

		void Execute(ID3D11DeviceContext* ctx) override;
		void Clear() override;

	private:
		void ExecuteGeometry(ID3D11DeviceContext* ctx);
		void ExecuteText(ID3D11DeviceContext* ctx);

		std::vector<RenderCommand> m_queue;
		std::vector<TextCommand>   m_textQueue;

		// b0: PerFrame (viewProj) -- Ortho projection
		// b1: PerObject (world)   -- per UI element transform
		ConstantBuffer m_cbPerFrame;
		ConstantBuffer m_cbPerObject;

		// UI uses Y-down ortho projection, so culling is disabled
		RefCom<ID3D11RasterizerState> m_noCullRS;

		// SpriteBatch for text rendering
		std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	};
}
