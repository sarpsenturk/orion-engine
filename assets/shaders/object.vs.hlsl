
[[vk::binding(0, 0)]]
cbuffer Scene {
    row_major float4x4 _viewProjection;
};

struct VsInput {
    float3 position: POSITION;
};

struct VsOutput {
    float4 position: SV_Position;
};

VsOutput vs_main(VsInput input)
{
    VsOutput output;
    output.position = float4(input.position, 1.0);
    return output;
}
