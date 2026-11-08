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
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD;
};

VS_OUTPUT main(float4 pos : POSITION, float3 nor : NORMAL,float3 tangent : TANGENT, float2 uv : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = mul(pos, world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    output.normal = mul(nor, (float3x3) world);
    output.tangent = normalize(mul(tangent, (float3x3) world));
    output.binormal = cross(output.normal, output.tangent);
    output.uv = uv;
    return output;
}
