struct PsInput {
   float4 color : COLOR;
};

float4 ps_main(PsInput input) : SV_Target
{
   return input.color;
}