#pragma once

namespace Render
{
	// 게임 오브젝트가 매 프레임 RenderPipeline::Submit()으로 제출하는 경량 구조체
	struct RenderCommand
	{
		class Geometry* geometry = nullptr;
		class Material* material = nullptr;
		float4x4        world    = {};
		float4x4        viewProj = {};
	};
}
