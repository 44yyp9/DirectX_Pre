cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 light;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : NORMAL;
    float2 uv : TEXCOORD;
};

VS_OUTPUT main(float4 pos : POSITION, float3 nor : NORMAL, float4 col : COLOR,float2 uv : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = mul(pos, world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    float3 normal = mul(nor, (float3x3) world);
    normal = normalize(normal);
    float lightAmount = saturate(dot(normal, (float3) light));
    lightAmount = lightAmount * 0.5f + 0.5f;

    output.color = col * lightAmount;
    output.uv = uv;
    return output;
}
