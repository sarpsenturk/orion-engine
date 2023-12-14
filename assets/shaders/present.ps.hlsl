
struct PsInput {
    float2 uv : TEXCOORD;
};

Texture2D RendererOutput;
SamplerState RendererSampler;

float4 ps_main(PsInput input) : SV_Target
{
    return RendererOutput.Sample(RendererSampler, input.uv);
}
