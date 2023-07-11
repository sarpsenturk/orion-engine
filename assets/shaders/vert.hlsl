struct VsOutput {
   float4 position : SV_Position;
   float4 color : COLOR;
};

VsOutput main(float3 position : POSITION, float4 color : COLOR)
{
   VsOutput output;
   output.position = float4(position, 1.0);
   output.color = color;
   return output;
}
