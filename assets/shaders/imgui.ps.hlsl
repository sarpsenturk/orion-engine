struct PsInput {
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

[[vk::binding(0)]]
Texture2D font_texture : register(t0);
[[vk::binding(1)]]
SamplerState font_sampler : register(s0);

float4 ps_main(PsInput input) : SV_Target
{
    return font_texture.Sample(font_sampler, input.uv) * input.color;
}
