struct PS_IN
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

struct PS_OUT
{
    float4 color : SV_TARGET0;
};

SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);
Texture2D Texture : register(t0);

PS_OUT main(PS_IN input)
{
    PS_OUT o;  
    
    o.color = Texture.Sample(LinearSampler, input.uv);
    
    if (0.05f > o.color.a)
        discard;
    
    return o;
}
