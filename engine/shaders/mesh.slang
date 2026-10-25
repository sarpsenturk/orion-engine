struct MeshRoot {
    float3* vertices;
    float4x4 mvp;
};

[vk::push_constant]
ConstantBuffer<MeshRoot> root;

struct VSOutput {
    float4 position : SV_Position;
}

[shader("vertex")]
VSOutput vsmain(uint vtx : SV_VertexID)
{
    let vertex = root.vertices[vtx];
    VSOutput output;
    output.position = mul(root.mvp, float4(vertex, 1.0));
    return output;
}

[shader("fragment")]
float4 fsmain() : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
