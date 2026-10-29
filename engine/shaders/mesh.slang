struct Material {
    float4 base_color;
}

struct MeshRoot {
    float3* vertices;
    Material* material;
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
    let material = root.material;
    return material.base_color;
}
