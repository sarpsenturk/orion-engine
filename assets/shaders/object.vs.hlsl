
[[vk::binding(0, 0)]]
cbuffer Scene {
    float4x4 _viewProjection;
};

struct Object {
    float4x4 transform;
};

[[vk::binding(0, 2)]]
StructuredBuffer<Object> Objects;

struct VsInput {
    float3 position: POSITION;
};

struct VsOutput {
    float4 position: SV_Position;
};

VsOutput vs_main(VsInput input, uint object_id : SV_InstanceID)
{
    VsOutput output;
    output.position = mul(_viewProjection, mul(Objects[object_id].transform, float4(input.position, 1.0)));
    return output;
}
