// Render a triangle using static vertex data and a simple fragment shader.

static const float4 vertices[3] = {
	float4(-0.5, -0.5, 0.0, 1.0),
	float4(0.5, -0.5, 0.0, 1.0),
	float4(0.0, 0.5, 0.0, 1.0),
};

[shader("vertex")]
float4 vsmain(uint vtx : SV_VertexID) : SV_Position
{
    return vertices[vtx];
}

[shader("fragment")]
float4 fsmain() : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}