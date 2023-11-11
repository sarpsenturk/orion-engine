struct VsInput {
   float3 position : POSITION;
   float4 color : COLOR;
   float2 uv : TEXCOORD;
};

struct VsOutput {
   float4 position : SV_Position;
   float4 color : COLOR;
   float2 uv : TEXCOORD;
};

cbuffer Scene {
    float4x4 view_proj;
};

VsOutput vs_main(VsInput input)
{
   VsOutput output;
   output.position = mul(float4(input.position, 1.0), view_proj);
   output.color = input.color;
   output.uv = input.uv;
   return output;
}
