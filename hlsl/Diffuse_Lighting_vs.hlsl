//***************************************************************************************
// Diffuse lighting vertex shader, hardcoded lighting in corresponding VS.
//***************************************************************************************

// Constant buffer.
cbuffer cbPerObject
{
	float4x4 gWorldViewProj; 
	float4x4 gWorldInverseTranspose;
};

struct VertexIn
{
	float3 Pos    : POSITION;
    float3 Normal : NORMAL;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	// Hardcoded.
	float3 DiffuseLightDirection = float3(1, 0, 0);
	float4 DiffuseColor = float4(1, 1, 1, 1);
	float DiffuseIntensity = 1.0;

	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
	float4 normal = mul(float4(vin.Normal, 0.0f), gWorldInverseTranspose);
	float lightIntensity = dot(normal.xyz, DiffuseLightDirection);
	vout.Color = saturate(DiffuseColor * DiffuseIntensity * lightIntensity);
    
    return vout;
}

