
struct PsInput {
    float2 uv : TEXCOORD;
};

float4 ps_main(PsInput input) : SV_Target
{
    return float4(input.uv, 0.0, 1.0);
}
