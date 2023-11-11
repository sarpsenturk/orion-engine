struct VsInput {
    float2 position : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct VsOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct SceneConstants {
    row_major float4x4 projection;
};

[[vk::push_constant]]
SceneConstants scene_data;

VsOutput vs_main(VsInput input)
{
    VsOutput output;
    output.position = mul(float4(input.position, -.5f, 1.f), scene_data.projection);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}