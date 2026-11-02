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
}

struct MeshRoot {
    float3* vertices;
    float2* uvs;
    Material* material;
    float4x4 mvp;
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
    let vertex = root.vertices[vtx];
    VSOutput output;
    output.position = mul(root.mvp, float4(vertex, 1.0));
    output.uv = root.uvs[vtx];
    return output;
}

[shader("fragment")]
float4 fsmain(VSOutput input) : SV_Target
{
    let material = root.material;
    float4 color = material.base_color;
    if (material.base_color_texture != ~0u) {
        color *= sampled_images[material.base_color_texture].Sample(samplers[0], input.uv);
    }
    return color;
}
