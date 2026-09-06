cbuffer ColorBuffer : register(b0)
{
    float4 g_color;
};

struct PS_IN
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

struct PS_OUT
{
    float4 color : SV_TARGET0;
};

PS_OUT main(PS_IN input)
{
    PS_OUT o;
    o.color = g_color;
    return o;
}
