
struct PsInput {
    float2 uv : TEXCOORD;
};

[[vk::binding(0, 0)]]
Texture2D _renderOutput;
[[vk::binding(1, 0)]]
SamplerState _presentSampler;

float4 ps_main(PsInput input) : SV_Target
{
    return _renderOutput.Sample(_presentSampler, input.uv);
}
