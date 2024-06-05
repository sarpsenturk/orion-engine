
[[vk::binding(0, 1)]]
cbuffer MaterialData {
    float4 color;
};

[[vk::binding(1, 1)]]
Texture2D _Texture;
[[vk::binding(2, 1)]]
SamplerState _Sampler;

struct PSInput {
    float2 uv : TEXCOORD;
};

float4 ps_main(PSInput input): SV_Target
{
    return _Texture.Sample(_Sampler, input.uv) * color;
}
