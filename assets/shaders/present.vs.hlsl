struct VsOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

VsOutput vs_main(uint vertex_id : SV_VertexID)
{
    VsOutput output;
    output.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
    output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}
