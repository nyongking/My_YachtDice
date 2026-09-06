struct PS_IN
{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
};

cbuffer UIData : register(b0)
{
    float4 g_tintColor;
};

Texture2D    gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 main(PS_IN input) : SV_Target
{
    float4 texColor = gTexture.Sample(gSampler, input.texcoord);
    return texColor * g_tintColor;
}
