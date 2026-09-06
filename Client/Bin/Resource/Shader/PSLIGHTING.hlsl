// G-Buffer 입력
Texture2D    gAlbedo   : register(t0);  // RGB: Albedo,   A: SpecularStrength
Texture2D    gNormal   : register(t1);  // RGB: Normal,   A: Shininess (정규화)
Texture2D    gWorldPos : register(t2);  // RGB: WorldPos, A: unused

SamplerState gSampler  : register(s1);  // linear clamp

#define MAX_LIGHTS 64

struct LightData
{
    uint   type;        // 0=Directional, 1=Point, 2=Spot
    float3 color;
    float3 position;
    float  range;
    float3 direction;
    float  intensity;
    float  spotInner;   // cos(inner half-angle)
    float  spotOuter;   // cos(outer half-angle)
    float2 _pad;
};

cbuffer LightBuffer : register(b0)
{
    int       g_lightCount;
    float3    g_cameraPos;
    LightData g_lights[MAX_LIGHTS];
};

struct PS_IN
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

float4 main(PS_IN input) : SV_TARGET0
{
    float4 albedoSample = gAlbedo.Sample(gSampler, input.uv);
    float4 normalSample = gNormal.Sample(gSampler, input.uv);

    float3 albedo   = albedoSample.rgb;
    float  specStr  = albedoSample.a;
    float3 normal   = normalize(normalSample.xyz);
    float  shininess = normalSample.a * 256.0;
    float3 worldPos = gWorldPos.Sample(gSampler, input.uv).xyz;

    float3 V = normalize(g_cameraPos - worldPos);

    float3 result = albedo * 0.1f;  // ambient

    for (int i = 0; i < g_lightCount; ++i)
    {
        LightData light = g_lights[i];

        float3 L     = float3(0, 0, 0);
        float  atten = 1.f;

        if (light.type == 0)  // Directional
        {
            L = normalize(-light.direction);
        }
        else if (light.type == 1)  // Point
        {
            float3 toLight = light.position - worldPos;
            float  dist    = length(toLight);
            if (dist >= light.range)
                continue;
            L     = toLight / dist;
            atten = 1.f - saturate(dist / light.range);
        }
        else if (light.type == 2)  // Spot
        {
            float3 toLight = light.position - worldPos;
            float  dist    = length(toLight);
            if (dist >= light.range)
                continue;
            L     = toLight / dist;
            atten = 1.f - saturate(dist / light.range);
            float spotDot    = dot(-L, normalize(light.direction));
            float spotFactor = smoothstep(light.spotOuter, light.spotInner, spotDot);
            atten *= spotFactor;
        }

        float NdotL = max(dot(normal, L), 0.f);

        // Diffuse
        float3 diffuse = albedo * light.color * light.intensity * NdotL * atten;

        // Blinn-Phong Specular
        float3 H     = normalize(L + V);
        float  NdotH = max(dot(normal, H), 0.f);
        float  spec  = pow(NdotH, max(shininess, 1.0));
        float3 specular = light.color * light.intensity * spec * specStr * atten;

        result += diffuse + specular;
    }

    return float4(result, 1.f);
}
