// Bindless descriptor arrays; binding 0 in each set matches the GPU layout.
[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D<float4> sampled_images[];

[[vk::binding(0, 2)]]
RWTexture2D<float4> storage_images[];

struct Material {
    float4 base_color;
    // Index into sampled_images; ~0u means no texture.
    uint base_color_texture;
    // Match the 32-byte CPU material array stride.
    uint padding[3];
}

struct MeshRecord {
    float3* positions;
    float2* uvs;
    float3* normals;
    uint index_count;
    uint first_index;
};

struct ViewData {
    float4x4 view;
    float4x4 projection;
    float4x4 view_projection;
    float4 camera_world_position;
};

struct MeshRoot {
    MeshRecord* meshes;
    Material* materials;
    ViewData* view;
    uint mesh_index;
    uint material_index;
    float4x4 model;
};

[vk::push_constant]
ConstantBuffer<MeshRoot> root;

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
}

[shader("vertex")]
VSOutput vsmain(uint vtx : SV_VertexID)
{
    let mesh = root.meshes[root.mesh_index];
    let vertex = mesh.positions[vtx];
    VSOutput output;
    let world_position = mul(root.model, float4(vertex, 1.0));
    output.position = mul(root.view->view_projection, world_position);
    output.uv = mesh.uvs[vtx];
    return output;
}

[shader("fragment")]
float4 fsmain(VSOutput input) : SV_Target
{
    let material = root.materials[root.material_index];
    float4 color = material.base_color;
    color *= sampled_images[material.base_color_texture].Sample(samplers[0], input.uv);
    return color;
}
