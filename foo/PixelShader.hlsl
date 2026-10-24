Texture2D g_texture : register(t0);

SamplerState g_sampler : register(s0);

cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 light;
};

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 lightDir = normalize(light.xyz);
    float lightAmount = saturate(dot(normal, lightDir));
    
    float ambient = 0.5f;
    lightAmount = saturate(lightAmount + ambient);

    float4 texColor = g_texture.Sample(g_sampler, input.uv);
    return float4(texColor.rgb * lightAmount,texColor.a);
}