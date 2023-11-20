struct VsOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

static const float4 positions[3] = {
    float4(-0.5f, -0.5f, 0.f, 1.f),
    float4(0.0f, 0.5f, 0.f, 1.f),
    float4(0.5f, -0.5f, 0.f, 1.f),
};

static const float4 colors[3] = {
    float4(1.f, 0.f, 0.f, 1.f),
    float4(0.f, 1.f, 0.f, 1.f),
    float4(0.f, 0.f, 1.f, 1.f),
};

VsOutput vs_main(uint vertex_id : SV_VertexID)
{
    VsOutput output;
    output.position = positions[vertex_id];
    output.color = colors[vertex_id];
    return output;
}
