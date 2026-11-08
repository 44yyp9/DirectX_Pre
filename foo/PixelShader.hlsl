Texture2D g_texture : register(t0);

Texture2D g_normalMap : register(t1);

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
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3 normalTex = g_normalMap.Sample(g_sampler, input.uv).rgb;
    float3 localNormal = normalTex * 2.0f - 1.0f;
    
    float3 T = normalize(input.tangent);
    float3 B = normalize(input.binormal);
    float3 N = normalize(input.normal);
    
    //グラム・シュミットの直交化は必要に応じて実装する
    
    float3x3 tbnMatrix = float3x3(T, B, N);
    
    float3 normal = normalize(mul(localNormal, tbnMatrix));
    
    float3 lightDir = normalize(light.xyz);
    float lightAmount = saturate(dot(normal, lightDir));
    
    float ambient = 0.5f;
    lightAmount = saturate(lightAmount + ambient);

    float4 texColor = g_texture.Sample(g_sampler, input.uv);
    return float4(texColor.rgb * lightAmount,texColor.a);
}