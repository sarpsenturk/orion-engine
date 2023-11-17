struct VsInput {
   float3 position : POSITION;
   float4 color : COLOR;
};

struct VsOutput {
   float4 position : SV_Position;
   float4 color : COLOR;
};

VsOutput vs_main(VsInput input)
{
   VsOutput output;
   output.position = float4(input.position, 1.0);
   output.color = input.color;
   return output;
}
