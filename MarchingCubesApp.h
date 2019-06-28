//***************************************************************************************
// Simple Direct3D demo application class.  
// Make sure you link: d3d11.lib D3DCompiler.lib
//***************************************************************************************

#ifndef MARCHINGCUBESAPP_H
#define MARCHINGCUBESAPP_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <d3d11.h>
#include <dxgi.h>

#include <string>

#include "Camera.h"
#include "GameTimer.h"

class MarchingCubesMesh;
class BallScalarField;
class MetaballScalarField;

class MarchingCubesApp
{
public:
	MarchingCubesApp(HINSTANCE hInstance);
	virtual ~MarchingCubesApp();
	
	HINSTANCE AppInst()const;
	HWND      MainWnd()const;
	float     AspectRatio()const;
	
	int Run();
 
	virtual bool Init();
	virtual void OnResize(); 
	virtual void UpdateScene(float dt);
	virtual void DrawScene(); 
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	void CalculateFrameStats();
	float RandFloat(float a);

protected:

	HINSTANCE mhAppInst;
	HWND      mhMainWnd;
	bool      mAppPaused;
	bool      mMinimized;
	bool      mMaximized;
	bool      mResizing;
	UINT      m4xMsaaQuality;

	GameTimer mTimer;

	ID3D11Device * md3dDevice;
	ID3D11DeviceContext * md3dImmediateContext;
	IDXGISwapChain * mSwapChain;
	ID3D11Texture2D * mDepthStencilBuffer;
	ID3D11RenderTargetView * mRenderTargetView;
	ID3D11DepthStencilView * mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring mMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;
	int mClientWidth;
	int mClientHeight;
	bool mEnable4xMsaa;

	// Mouse position
	POINT mLastMousePos;
	Camera mCam;

	// What we are drawing.
	BallScalarField * mBallField;
	MarchingCubesMesh * mSphereMesh;
	MetaballScalarField * mMetaballField;
	MarchingCubesMesh * mMetaballMesh;

	// For animation.
	float dIso;
	float * dX;
	float * dY;
	float * dZ;
};

#endif // MARCHINGCUBESAPP_H
