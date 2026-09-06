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

cbuffer WorldViewBuffer : register(b0)
{
    row_major float4x4 viewMatrix;
    row_major float4x4 projMatrix;
    row_major float4x4 viewprojMatrix;
};

cbuffer MatrixBuffer : register(b1)
{
    row_major float4x4 worldMatrix;
};

cbuffer InstanceBuffer : register(b2)
{
    float2 scale;
    float2 rotation;
    float3 position;

    float4 uvRect;
    float4 color;
    float pad;
}


VS_OUT main(VS_IN input)
{
    VS_OUT output;
    
    output.position = mul(float4(input.position, 1.0f), mul(worldMatrix, viewprojMatrix));
    return output;
}