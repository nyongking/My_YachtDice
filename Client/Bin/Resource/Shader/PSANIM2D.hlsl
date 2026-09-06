struct PS_IN
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 color : SV_TARGET0;
};

SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);
Texture2D DiffuseTexture : register(t0);

PS_OUT main(PS_IN input)
{
    PS_OUT Out = (PS_OUT) 0;
    
    Out.color = DiffuseTexture.Sample(LinearSampler, input.texcoord);
    
    if (0.05f > Out.color.a)
        discard;
    
    return Out;

}