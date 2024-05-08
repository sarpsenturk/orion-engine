
[[vk::binding(0, 1)]]
SamplerState _sampler;
[[vk::binding(1, 1)]]
Texture2D _textures[4096];

[[vk::push_constant]]
struct { int index; } _texture;

struct PSInput {
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 ps_main(PSInput input) : SV_Target
{
    return input.color * _textures[_texture.index].Sample(_sampler, input.uv);
}
