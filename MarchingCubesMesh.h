#ifndef MARCHINGCUBESMESH_H
#define MARCHINGCUBESMESH_H
//***************************************************************************************
// Class that builds and/or draws a mesh created by the marching cubes algorithm over
// the passed scalar field.
//***************************************************************************************

#include <vector>
#include <windows.h>
#include <DirectXMath.h>


class ScalarField;
struct ID3D11Device;
struct ID3D11DeviceContext;

// Basic 32bit vertex structure.
struct MC_VTX
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
};

struct MC_CONST {
    DirectX::XMFLOAT4X4 mWorldViewProj;
	DirectX::XMFLOAT4X4 mWorldInverseTranspose;
};

class MarchingCubesMesh
{
public:
	MarchingCubesMesh(ScalarField * scalarField, float isoLevel, bool gpuTesselation);
	virtual ~MarchingCubesMesh();

	void SetIsoLevel(float isoLevel);
	float GetIsoLevel() { return mIsoLevel; };

	bool Init(ID3D11Device* device);
	bool Destroy();

	bool Setup(ID3D11DeviceContext* deviceContext);
	bool Draw(ID3D11DeviceContext* deviceContext, const DirectX::XMFLOAT4X4 * mView, const DirectX::XMFLOAT4X4 * mProj);

protected:
	// The scalar field to be represented.
	ScalarField * mScalarField;

	// The iso level to be tesselated.
	float mIsoLevel;

	// Mesh definition, for non GPU tesselation.
	std::vector<float> mVertexCoors;
	std::vector<MC_VTX> mVertices;
	std::vector<UINT> mIndices;
	MC_CONST mConstant;

	// World matrix, defines the position of this object.
	DirectX::XMFLOAT4X4 mWorld;

	// The device used to draw.
	ID3D11Device* md3dDevice;

	// Corresponding buffers.
	ID3D11Buffer * mVBuffer;  // Vertex buffer
	ID3D11Buffer * mIBuffer;  // Index buffer
	ID3D11Buffer * mCBuffer;  // Constant buffer

	ID3D11InputLayout* mInputLayout;

	ID3D11VertexShader* mVS;
	ID3D11PixelShader* mPS;

private:
    MarchingCubesMesh( const MarchingCubesMesh& );
    const MarchingCubesMesh& operator=( const MarchingCubesMesh& );
};

#endif
