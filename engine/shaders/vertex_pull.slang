// Pull tightly packed float3 positions from the vertex buffer.

struct DrawRoot {
    float3* vertices;
};

[vk::push_constant]
ConstantBuffer<DrawRoot> root;

[shader("vertex")]
float4 vsmain(uint vtx : SV_VertexID) : SV_Position
{
    return float4(root.vertices[vtx], 1.0);
}

[shader("fragment")]
float4 fsmain() : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
