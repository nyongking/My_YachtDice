cbuffer PerFrame  : register(b0) { row_major float4x4 g_viewProj; };
cbuffer PerObject : register(b1) { row_major float4x4 g_world; };

struct VS_IN  { float3 pos : POSITION; float4 col : COLOR; };
struct VS_OUT { float4 pos : SV_Position; float4 col : COLOR; };

VS_OUT main(VS_IN i)
{
    VS_OUT o;
    float4 worldPos = mul(float4(i.pos, 1.f), g_world);
    o.pos = mul(worldPos, g_viewProj);
    o.col = i.col;
    return o;
}
