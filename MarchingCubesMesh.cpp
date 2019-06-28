#include <limits>

#include <d3d11.h>
#include <D3Dcompiler.h>

#include "Globals.h"

#include "MarchingCubesMesh.h"
#include "MarchingCubesPolygonizer.h"


MarchingCubesMesh::MarchingCubesMesh(ScalarField * scalarField, float isoLevel, bool gpuTesselation)
	: mIsoLevel(std::numeric_limits<float>::quiet_NaN()),
	  mVBuffer(NULL),
	  mIBuffer(NULL),
	  mCBuffer(NULL),
	  mVS(NULL),
	  mPS(NULL),
	  mInputLayout(NULL)
{
	DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);

	mScalarField = scalarField;
	SetIsoLevel(isoLevel);
}


MarchingCubesMesh::~MarchingCubesMesh()
{
	Destroy();
}


void MarchingCubesMesh::SetIsoLevel(float isoLevel)
{
	if (isoLevel == mIsoLevel) return; // Nothing to do
	mIsoLevel = isoLevel;

	// Free buffers if needed. But only buffers.
	// They'll be recreated in the setup.
	if (mVBuffer != NULL)
	{
		mVBuffer->Release();
		mVBuffer = NULL;
	}
	if (mIBuffer != NULL)
	{
		mIBuffer->Release();
		mIBuffer = NULL;
	}

	// Recompute the mesh.
	mVertexCoors.clear();
	MarchingCubesPolygonizer polygonizer(mScalarField);
	polygonizer.Polygonize(isoLevel, &mVertexCoors);

	int nbTrias = mVertexCoors.size() / 9;
	mVertices.clear();
	mIndices.clear();
	for (u_int i = 0; i<mVertexCoors.size();)
	{
		mIndices.push_back(i/3);
		MC_VTX v1;
		v1.Pos.x = mVertexCoors[i++];
		v1.Pos.y = mVertexCoors[i++];
		v1.Pos.z = mVertexCoors[i++];
		mIndices.push_back(i / 3);
		MC_VTX v2;
		v2.Pos.x = mVertexCoors[i++];
		v2.Pos.y = mVertexCoors[i++];
		v2.Pos.z = mVertexCoors[i++];
		mIndices.push_back(i / 3);
		MC_VTX v3;
		v3.Pos.x = mVertexCoors[i++];
		v3.Pos.y = mVertexCoors[i++];
		v3.Pos.z = mVertexCoors[i++];

		float v1v2X = v2.Pos.x - v1.Pos.x;
		float v1v2Y = v2.Pos.y - v1.Pos.y;
		float v1v2Z = v2.Pos.z - v1.Pos.z;

		float v1v3X = v3.Pos.x - v1.Pos.x;
		float v1v3Y = v3.Pos.y - v1.Pos.y;
		float v1v3Z = v3.Pos.z - v1.Pos.z;

		float nX = v1v2Z*v1v3Y - v1v2Y*v1v3Z;
		float nY = v1v2X*v1v3Z - v1v2Z*v1v3X;
		float nZ = v1v2Y*v1v3X - v1v2X*v1v3Y;

		float norm = sqrt(nX * nX + nY*nY + nZ*nZ);
		nX /= norm;
		nY /= norm;
		nZ /= norm;

		v1.Normal.x = nX;
		v1.Normal.y = nY;
		v1.Normal.z = nZ;

		v2.Normal.x = nX;
		v2.Normal.y = nY;
		v2.Normal.z = nZ;

		v3.Normal.x = nX;
		v3.Normal.y = nY;
		v3.Normal.z = nZ;

		mVertices.push_back(v1);
		mVertices.push_back(v2);
		mVertices.push_back(v3);
	}
}


bool MarchingCubesMesh::Init(ID3D11Device* device)
{
	md3dDevice = device;
	md3dDevice->AddRef();

	// Load basic color vertex shader.
	ID3DBlob* VS_Buffer;
	HRESULT hr = CompileShaderFromFile(L"hlsl\\Diffuse_Lighting_vs.hlsl", "VS","vs_5_0", &VS_Buffer); //D3DReadFileToBlob(L"Diffuse_Lighting_vs.cso", &VS_Buffer);
	md3dDevice->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &mVS);
	
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
	md3dDevice->CreateInputLayout(vertexDesc, 2, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &mInputLayout);

	// We're done with the vertex shader blob.
	VS_Buffer->Release();
	VS_Buffer = NULL;

	// Load basic color pixel shader.
	ID3DBlob* PS_Buffer;
	hr = CompileShaderFromFile(L"hlsl\\Diffuse_Lighting_ps.hlsl", "PS","ps_5_0", &PS_Buffer ); //D3DReadFileToBlob(L"Diffuse_Lighting_ps.cso", &PS_Buffer);
	md3dDevice->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &mPS);

	// We're done with the pixel shader blob.
	PS_Buffer->Release();
	PS_Buffer = NULL;

	return true;
}


bool MarchingCubesMesh::Destroy()
{
	if (mVBuffer != NULL)
	{
		mVBuffer->Release();
		mVBuffer = NULL;
	}
	if (mIBuffer != NULL)
	{
		mIBuffer->Release();
		mIBuffer = NULL;
	}
	if (mCBuffer != NULL)
	{
		mCBuffer->Release();
		mCBuffer = NULL;
	}
	if (mInputLayout != NULL)
	{
		mInputLayout->Release();
		mInputLayout = NULL;
	}
	if (mVS != NULL)
	{
		mVS->Release();
		mVS = NULL;
	}
	if (mPS != NULL)
	{
		mPS->Release();
		mPS = NULL;
	}
	if (md3dDevice != NULL)
	{
		md3dDevice->Release();
		md3dDevice = NULL;
	}
	return true;
}


bool MarchingCubesMesh::Setup(ID3D11DeviceContext* deviceContext)
{
	if (mVBuffer == NULL)
	{
		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(MC_VTX) * mVertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = mVertices.data();
		md3dDevice->CreateBuffer(&vbd, &vinitData, &mVBuffer);
	}

	if (mIBuffer == NULL)
	{
		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * mIndices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = mIndices.data();
		md3dDevice->CreateBuffer(&ibd, &iinitData, &mIBuffer);
	}

	if (mCBuffer == NULL)
	{
		CD3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof(MC_CONST);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = 0;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &mConstant;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		md3dDevice->CreateBuffer(&cbDesc, &InitData, &mCBuffer);
	}

	return true;
}


bool MarchingCubesMesh::Draw(ID3D11DeviceContext* deviceContext, const DirectX::XMFLOAT4X4 * mView, const DirectX::XMFLOAT4X4 * mProj)
{
	deviceContext->IASetInputLayout(mInputLayout);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MC_VTX);
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &mVBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(mIBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set up the vertex shader stage.
    deviceContext->VSSetShader(mVS, NULL, 0);

	// Set constants
	DirectX::XMMATRIX world = XMLoadFloat4x4(&mWorld);
	DirectX::XMMATRIX view  = XMLoadFloat4x4(mView);
	DirectX::XMMATRIX proj  = XMLoadFloat4x4(mProj);
	DirectX::XMMATRIX worldViewProj = DirectX::XMMatrixTranspose(world*view*proj);
	DirectX::XMMATRIX worldInverseTranspose = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(NULL, XMLoadFloat4x4(&mWorld)));

	DirectX::XMStoreFloat4x4(&mConstant.mWorldViewProj, worldViewProj);
	DirectX::XMStoreFloat4x4(&mConstant.mWorldInverseTranspose, worldInverseTranspose);
	deviceContext->UpdateSubresource(mCBuffer, 0, NULL, &mConstant, 0, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &mCBuffer);

    // Set up the pixel shader stage.
    deviceContext->PSSetShader(mPS, NULL, 0);

    // Calling Draw tells Direct3D to start sending commands to the graphics device.
	deviceContext->DrawIndexed(mIndices.size(), 0, 0);

	return true;
}
