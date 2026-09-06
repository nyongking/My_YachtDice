cbuffer PerFrame : register(b0)
{
    row_major float4x4 g_viewProj;
};

cbuffer PerObject : register(b1)
{
    row_major float4x4 g_world;
};

struct VS_IN
{
    float3 pos     : POSITION;
    float3 normal  : NORMAL;
    float3 tangent : TANGENT;
    float2 uv      : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos      : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

VS_OUT main(VS_IN input)
{
    VS_OUT o;

    float4 worldPos   = mul(float4(input.pos, 1.f), g_world);
    o.pos             = mul(worldPos, g_viewProj);

    // 노말은 월드 공간으로 변환 (스케일 없다고 가정, 필요 시 InverseTranspose 사용)
    o.normal = normalize(mul(input.normal, (float3x3)g_world));
    o.uv     = input.uv;

    return o;
}
