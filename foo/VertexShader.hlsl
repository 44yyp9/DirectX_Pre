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
    float3 lightDirSurface : TESSFACTOR01;
    float2 uv : TEXCOORD;
};

VS_OUTPUT main(float4 pos : POSITION, float3 nor : NORMAL,float3 tangent : TANGENT, float2 uv : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.pos = mul(pos, world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    float3 N = normalize(mul(nor, (float3x3) world));
    float3 T = normalize(mul(tangent, (float3x3) world));
    float3 B = cross(N, T);
    
    float3x3 tbnMatrix = float3x3(T, B, N);
    //ãtçsóÒÇ…Ç∑ÇÈ
    float3x3 invTbnMatrix = transpose(tbnMatrix);
    
    float3 lightDirWorld = normalize(light.xyz);
    output.lightDirSurface = mul(lightDirWorld, invTbnMatrix);
    
    output.uv = uv;
    return output;
}
