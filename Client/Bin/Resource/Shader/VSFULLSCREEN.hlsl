// 버텍스 버퍼 없이 SV_VertexID만으로 풀스크린 삼각형 생성
// Draw(3, 0) 호출로 화면 전체를 커버하는 삼각형 1개를 그린다

struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

VS_OUT main(uint vertID : SV_VertexID)
{
    VS_OUT o;

    // vertID 0,1,2 → UV (0,0), (2,0), (0,2)
    o.uv  = float2((vertID << 1) & 2, vertID & 2);

    // NDC: UV [0,1] → [-1,+1], Y 반전 (D3D UV 원점은 좌상단)
    o.pos = float4(o.uv.x * 2.f - 1.f, 1.f - o.uv.y * 2.f, 0.f, 1.f);

    return o;
}
