struct VS_IN
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUT
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

cbuffer cbPerFrame : register(b0)
{
    row_major float4x4 g_viewProj;
};

cbuffer cbPerObject : register(b1)
{
    row_major float4x4 g_world;
};

cbuffer UVRect : register(b2)
{
    float4 g_uvRect; // (u0, v0, uWidth, vHeight)
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    float4 worldPos = mul(float4(input.position, 1.0f), g_world);
    output.position = mul(worldPos, g_viewProj);
    output.texcoord = g_uvRect.xy + input.texcoord * g_uvRect.zw;

    return output;
}
