// Matches GpuView in orion/renderer/view.hpp; world coordinates use a zero render origin.
struct ViewData {
    float4x4 view;
    float4x4 projection;
    float4x4 view_projection;
    float4 camera_world_position;
};
