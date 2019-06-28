//***************************************************************************************
// Diffuse lighting pixel shader, hardcoded lighting.
//***************************************************************************************

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

float4 PS(VertexOut pin) : SV_Target
{
	// Hardcoded.
	float4 AmbientColor = float4(1, 1, 1, 1);
	float AmbientIntensity = 0.1;

	return saturate(pin.Color + AmbientColor * AmbientIntensity);
}
