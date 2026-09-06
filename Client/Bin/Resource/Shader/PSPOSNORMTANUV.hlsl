struct PS_IN
{
    float4 pos    : SV_Position;
    float3 normal : NORMAL;
    float2 uv     : TEXCOORD0;
};

struct PS_OUT
{
    float4 color : SV_TARGET0;
};

PS_OUT main(PS_IN input)
{
    PS_OUT o;

    // 노말을 [0,1] 범위로 변환해 색상으로 시각화 — 메시 확인용
    float3 n = normalize(input.normal) * 0.5f + 0.5f;
    o.color = float4(n, 1.f);
    //o.color = float4(1.f, 0.f, 1.f, 1.f);
    
    return o;
}
