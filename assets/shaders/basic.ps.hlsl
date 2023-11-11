struct PsInput {
   float4 color : COLOR;
   float2 uv : TEXCOORD;
};

Texture2D texture : register(t0);
SamplerState texture_sampler : register(s0);

float4 ps_main(PsInput input) : SV_Target
{
   return texture.Sample(texture_sampler, input.uv) * input.color;
}