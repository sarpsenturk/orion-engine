
[[vk::binding(0, 1)]]
cbuffer MaterialData {
    float4 color;
};

float4 ps_main(): SV_Target
{
    return color;
}
