// Vertex shader
struct VsOutput {
   float4 position : SV_Position;
   float4 color : COLOR;
};

cbuffer Scene {
    float4x4 view_proj;
};

VsOutput vs_main(float3 position : POSITION, float4 color : COLOR)
{
   VsOutput output;
   output.position = mul(float4(position, 1.0), view_proj);
   output.color = color;
   return output;
}

// Fragment shader
float4 fs_main(float4 color : COLOR) : SV_Target
{
   return color;
}

