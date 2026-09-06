#pragma once

namespace Render
{
	static constexpr int MAX_LIGHTS = 64;

	enum class LightType : unsigned int
	{
		Directional = 0,
		Point       = 1,
		Spot        = 2,
	};

	// HLSL cbuffer 패킹 규칙(16바이트 정렬)과 1:1 대응
	// 총 64바이트 (float4 × 4)
	//
	// cbuffer LightData : register(b2)
	// {
	//     int    g_lightType;   float3 g_lightColor;   // 16
	//     float3 g_position;    float  g_range;         // 16
	//     float3 g_direction;   float  g_intensity;     // 16
	//     float  g_spotInner;   float  g_spotOuter;   float2 _pad; // 16
	// }

	struct alignas(16) LightData
	{
		unsigned int type      = static_cast<unsigned int>(LightType::Directional);
		float3  color     = { 1.f, 1.f, 1.f };  // linear RGB
		float3  position  = { 0.f, 0.f, 0.f };  // Point / Spot
		float   range     = 10.f;               // Point / Spot (0 = 무한)
		float3  direction = { 0.f, -1.f, 0.f }; // Directional / Spot
		float   intensity = 1.f;
		float   spotInner = 0.f;                 // cos(inner half-angle)
		float   spotOuter = 0.f;                 // cos(outer half-angle)
		float   _pad[2]   = {};
	};

	// LightComponent가 매 프레임 RenderPipeline::SubmitLight()으로 제출하는 경량 구조체
	struct LightCommand
	{
		LightData data;
	};
}
