// Bindless descriptor arrays; binding 0 in each set matches the GPU layout.
[[vk::binding(0, 0)]]
SamplerState samplers[];

[[vk::binding(0, 1)]]
Texture2D<float4> sampled_images[];

[[vk::binding(0, 2)]]
RWTexture2D<float4> storage_images[];
