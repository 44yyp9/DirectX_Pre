Texture2D g_texture : register(t0);

SamplerState g_sampler : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
    float2 uv : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float4 texColor = g_texture.Sample(g_sampler, input.uv);
    return texColor;
}