static const float3 vertices[] = {
    float3(-0.5, -0.5, 0.0),
    float3(0.5, -0.5, 0.0),
    float3(0.0, 0.5, 0.0),
};

float4 main(uint vertex_id: SV_VertexID): SV_Position
{
    return float4(vertices[vertex_id], 1.0);
}
