
struct QuadData {
    float3 position;
    float4 color;
};

// StructuredBuffer<QuadData> Quads : register(t0);

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

static const float3 vertex_positions[] = {
    float3(-.5f, .5f, 0.f),
    float3(0.5f, .5f, 0.f),
    float3(0.5f, -.5f, 0.f),
    float3(0.5f, -.5f, 0.f),
    float3(-.5f, -.5f, 0.f),
    float3(-.5f, .5f, 0.f),
};

VSOutput vs_main(uint vertex_id : SV_VertexID)
{
    uint vertex_index = vertex_id % 6;
    // uint quad_index = vertex_id / 6;
    // QuadData quad_data = Quads[quad_index];

    VSOutput output;
    output.position = float4(vertex_positions[vertex_index], 1.0f);
    // output.position += quad_data.position;
    // output.color = quad_data.color;
    output.color = float4(1.f, 1.f, 1.f, 1.f);
    return output;
}

float4 ORION_PS_MAIN(float4 color : COLOR) : SV_Target
{
    return color;
}
