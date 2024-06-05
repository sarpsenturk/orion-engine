struct PsInput {
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

[[vk::binding(0)]]
Texture2D _FontTexture : register(t0);
[[vk::binding(1)]]
SamplerState _FontSampler : register(s0);

float4 ps_main(PsInput input) : SV_Target
{
    return _FontTexture.Sample(_FontSampler, input.uv) * input.color;
}
