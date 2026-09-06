#include "RenderPch.h"
#include "UIPass.h"

#include "RenderTargetGroup.h"
#include "ShaderGroup.h"
#include "Material.h"
#include "Geometry.h"
#include "Renderer.h"

#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"

namespace Render
{
	UIPass::~UIPass() = default;

	bool UIPass::Initialize(ID3D11Device* device)
	{
		if (!device)
			return false;

		if (!m_cbPerFrame.Create(device, sizeof(float4x4), nullptr, D3D11_USAGE_DYNAMIC))
			return false;

		if (!m_cbPerObject.Create(device, sizeof(float4x4), nullptr, D3D11_USAGE_DYNAMIC))
			return false;

		// Y-down ortho projection flips winding, so disable culling
		D3D11_RASTERIZER_DESC rsDesc = {};
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_NONE;
		rsDesc.DepthClipEnable = TRUE;
		if (FAILED(device->CreateRasterizerState(&rsDesc, m_noCullRS.GetAddressOf())))
			return false;

		// Create SpriteBatch for text rendering
		RefCom<ID3D11DeviceContext> ctx;
		device->GetImmediateContext(ctx.GetAddressOf());
		if (ctx)
			m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(ctx.Get());

		return true;
	}

	void UIPass::Submit(const RenderCommand& cmd)
	{
		m_queue.push_back(cmd);
	}

	void UIPass::SubmitText(const TextCommand& cmd)
	{
		m_textQueue.push_back(cmd);
	}

	void UIPass::Execute(ID3D11DeviceContext* ctx)
	{
		bool hasGeometry = !m_queue.empty();
		bool hasText     = !m_textQueue.empty();

		if (!hasGeometry && !hasText)
			return;

		if (m_renderTarget)
			m_renderTarget->Bind(ctx);

		BindPipelineState(ctx);

		if (hasGeometry)
			ExecuteGeometry(ctx);

		if (hasText)
			ExecuteText(ctx);

		RestorePipelineState(ctx);
	}

	void UIPass::ExecuteGeometry(ID3D11DeviceContext* ctx)
	{
		// Disable culling (Y-down projection reverses winding)
		ID3D11RasterizerState* prevRS = nullptr;
		ctx->RSGetState(&prevRS);
		ctx->RSSetState(m_noCullRS.Get());

		const float4x4* currentVP = nullptr;

		ShaderGroup* currentShader = nullptr;

		for (const auto& cmd : m_queue)
		{
			if (nullptr == cmd.geometry || nullptr == cmd.material)
				continue;

			// Update viewProj per command (Billboard uses 3D VP, UI uses ortho VP)
			if (!currentVP || memcmp(currentVP, &cmd.viewProj, sizeof(float4x4)) != 0)
			{
				m_cbPerFrame.Update(ctx, &cmd.viewProj, sizeof(float4x4));
				m_cbPerFrame.BindVS(ctx, 0);
				currentVP = &cmd.viewProj;
			}

			m_cbPerObject.Update(ctx, &cmd.world, sizeof(float4x4));
			m_cbPerObject.BindVS(ctx, 1);

			ShaderGroup* sg = cmd.material->GetShaderGroup();

			if (sg != currentShader)
			{
				if (sg)
					sg->BindShaderAndLayout(ctx);
				currentShader = sg;
			}

			if (false == cmd.material->BindMaterial(ctx))
				continue;

			cmd.geometry->BindAndDraw(ctx);
		}

		// Restore rasterizer state
		ctx->RSSetState(prevRS);
		if (prevRS) prevRS->Release();
	}

	void UIPass::ExecuteText(ID3D11DeviceContext* ctx)
	{
		if (!m_spriteBatch)
			return;

		// Save pipeline state that SpriteBatch will overwrite
		static constexpr UINT MAX_SAMPLERS = 4;
		ID3D11SamplerState* prevSamplers[MAX_SAMPLERS] = {};
		ctx->PSGetSamplers(0, MAX_SAMPLERS, prevSamplers);

		ID3D11RasterizerState* prevRS = nullptr;
		ctx->RSGetState(&prevRS);

		m_spriteBatch->Begin(
			DirectX::SpriteSortMode_Deferred,
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		for (const auto& cmd : m_textQueue)
		{
			auto* font = static_cast<DirectX::SpriteFont*>(cmd.font);
			if (!font || cmd.text.empty())
				continue;

			DirectX::XMFLOAT2 pos = { cmd.position.x, cmd.position.y };
			DirectX::XMVECTOR color = DirectX::XMVectorSet(
				cmd.color.x, cmd.color.y, cmd.color.z, cmd.color.w);

			// Calculate origin for alignment
			DirectX::XMFLOAT2 origin = { 0.f, 0.f };
			if (cmd.alignment != TextAlignment::Left)
			{
				DirectX::XMVECTOR measured = font->MeasureString(cmd.text.c_str());
				float textW = DirectX::XMVectorGetX(measured);
				if (cmd.alignment == TextAlignment::Center)
					origin.x = textW * 0.5f;
				else // Right
					origin.x = textW;
			}

			font->DrawString(
				m_spriteBatch.get(),
				cmd.text.c_str(),
				pos,
				color,
				cmd.rotation,
				origin,
				cmd.scale,
				DirectX::SpriteEffects_None,
				cmd.depth
			);
		}

		m_spriteBatch->End();

		// Restore pipeline state that SpriteBatch overwrote
		ctx->PSSetSamplers(0, MAX_SAMPLERS, prevSamplers);
		for (UINT i = 0; i < MAX_SAMPLERS; ++i)
			if (prevSamplers[i]) prevSamplers[i]->Release();

		ctx->RSSetState(prevRS);
		if (prevRS) prevRS->Release();
	}

	void UIPass::Clear()
	{
		m_queue.clear();
		m_textQueue.clear();
	}
}
