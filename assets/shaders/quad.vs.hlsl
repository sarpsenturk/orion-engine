
struct QuadData {
    row_major float4x4 transform;
    float4 color;
};

StructuredBuffer<QuadData> Quads : register(t0);

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
    uint quad_index = vertex_id / 6;
    QuadData quad_data = Quads[quad_index];

    VSOutput output;
    float3 position = vertex_positions[vertex_index];
    output.position = mul(float4(position, 1.0f), quad_data.transform);
    output.color = quad_data.color;
    return output;
}
