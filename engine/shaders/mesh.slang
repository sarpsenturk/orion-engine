// Render meshes with vertex pulling

[[vk::binding(0, 0)]]
SamplerState Sampler;

[[vk::binding(1, 0)]]
Texture2D Textures[];

struct Vertex {
    float3 position;
    float2 uv;
}

struct Scene {
    float4x4 view_proj;
}

struct Material {
    uint albedo_texture;
    uint sampler_idx;
}

struct RootConstants {
    Vertex* vertices;
    Scene* scene;
    Material* materials;
    uint material_idx;
    float4x4 model;
}

[[vk::push_constant]] ConstantBuffer<RootConstants> root;

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
}

[shader("vertex")]
VSOutput vsmain(uint vtx: SV_VulkanVertexID)
{
    let vertex = root.vertices[vtx];
    let scene = root.scene;
    VSOutput output;
    output.position = mul(scene.view_proj, mul(root.model, float4(vertex.position, 1.0)));
    output.uv = vertex.uv;
    return output;
}

[shader("fragment")]
float4 fsmain(VSOutput input) : SV_Target
{
    let material = root.materials[root.material_idx];
    return Textures[material.albedo_texture].Sample(Sampler, input.uv);
}
