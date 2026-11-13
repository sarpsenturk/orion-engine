import common.bindings;
import common.view;

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

struct GpuInstance {
    uint mesh_idx;
    uint material_idx;
    uint padding[2];
    float4x4 transform;
};

struct MeshRoot {
    MeshRecord* meshes;
    Material* materials;
    ViewData* view;
    GpuInstance* instances;
};

[vk::push_constant]
ConstantBuffer<MeshRoot> root;

struct VSOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    uint material : MATERIAL;
}

[shader("vertex")]
VSOutput vsmain(uint vtx : SV_VertexID, uint instance_id : SV_VulkanInstanceID)
{
    let instance = root.instances[instance_id];
    let mesh = root.meshes[instance.mesh_idx];
    let vertex = mesh.positions[vtx];
    VSOutput output;
    let world_position = mul(instance.transform, float4(vertex, 1.0));
    output.position = mul(root.view->view_projection, world_position);
    output.uv = mesh.uvs[vtx];
    output.material = instance.material_idx;
    return output;
}

[shader("fragment")]
float4 fsmain(VSOutput input) : SV_Target
{
    let material = root.materials[input.material];
    float4 color = material.base_color;
    color *= sampled_images[material.base_color_texture].Sample(samplers[0], input.uv);
    return color;
}
