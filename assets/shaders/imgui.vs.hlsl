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

struct Constants {
    float2 scale;
    float2 translation;
};

[[vk::push_constant]]
Constants _constants;

VsOutput vs_main(VsInput input)
{
    VsOutput output;
    output.position = float4(input.position * _constants.scale + _constants.translation, 0.1, 1.0);
    output.position.y *= -1;
    output.uv = input.uv;
    output.color = input.color;
    return output;
}