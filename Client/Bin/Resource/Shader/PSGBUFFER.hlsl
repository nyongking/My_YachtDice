Texture2D    gAlbedoTex  : register(t0);
Texture2D    gNormalTex  : register(t1);
SamplerState gSampler    : register(s0);

cbuffer MaterialData : register(b0)
{
    float4 g_albedoColor;
    float2 g_uvTiling;
    float2 g_uvOffset;
    float  g_shininess;
    float  g_specularStrength;
    float2 _matPad;
};

struct PS_IN
{
    float4 pos      : SV_Position;
    float3 worldPos : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float2 uv       : TEXCOORD1;
};

struct PS_OUT
{
    float4 albedo   : SV_TARGET0;  // RGB: Albedo,   A: Metallic
    float4 normal   : SV_TARGET1;  // RGB: Normal,   A: Roughness
    float4 worldPos : SV_TARGET2;  // RGB: WorldPos, A: unused
};

PS_OUT main(PS_IN input)
{
    PS_OUT o;

    float2 uv = input.uv * g_uvTiling + g_uvOffset;
    //float2 uv;
    //uv.x = g_uvOffset.x + input.uv.x * g_uvTiling.x;
    //uv.y = g_uvOffset.y + input.uv.y * g_uvTiling.y;
    
    float4 texColor = gAlbedoTex.Sample(gSampler, uv);
    float4 albedo   = lerp(g_albedoColor, texColor, texColor.a);

    // Normal mapping
    float3 N = normalize(input.normal);
    float4 normalSample = gNormalTex.Sample(gSampler, uv);

    float3 finalNormal;
    if (normalSample.a > 0)
    {
        float3 T = normalize(input.tangent - dot(input.tangent, N) * N);
        float3 B = cross(N, T);
        float3x3 TBN = float3x3(T, B, N);

        // 비표준 노멀맵(R에 Z 저장) 대응: BGR → XYZ 스위즐
        float3 tangentNormal = normalSample.bgr * 2.0 - 1.0;
        finalNormal = normalize(mul(tangentNormal, TBN));
    }
    else
    {
        finalNormal = N;
    }

    o.albedo   = float4(albedo.rgb, g_specularStrength);       // A = specularStrength
    o.normal   = float4(finalNormal, g_shininess / 256.0);    // A = shininess (정규화)
    o.worldPos = float4(input.worldPos, 1.f);

    return o;
}
