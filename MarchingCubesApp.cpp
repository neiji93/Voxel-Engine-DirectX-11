#include "MarchingCubesApp.h"
#include <WindowsX.h>
#include <sstream>
#include <cassert>

#include <DirectXColors.h>
#include <DirectXMath.h>

#include "MarchingCubesMesh.h"
#include "BallScalarField.h"
#include "MetaballScalarField.h"

using namespace DirectX;

namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	MarchingCubesApp* gd3dApp = 0;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return gd3dApp->MsgProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	MarchingCubesApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}


MarchingCubesApp::MarchingCubesApp(HINSTANCE hInstance)
:	mhAppInst(hInstance),
	mMainWndCaption(L"MarchingCubes Demo App"),
	md3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	mClientWidth(800),
	mClientHeight(600),
	mEnable4xMsaa(false),
	mhMainWnd(0),
	mAppPaused(false),
	mMinimized(false),
	mMaximized(false),
	mResizing(false),
	m4xMsaaQuality(0),
 
	md3dDevice(0),
	md3dImmediateContext(0),
	mSwapChain(0),
	mDepthStencilBuffer(0),
	mRenderTargetView(0),
	mDepthStencilView(0)
{
	ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));

	// Get a pointer to the application object so we can forward 
	// Windows messages to the object's window procedure through
	// the global window procedure.
	gd3dApp = this;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetLens(0.5f*XM_PI, 1.0f, 1.0f, 100.0f);
	mCam.Pitch(25.0f / 180.f*XM_PI);
	mCam.SetPosition(0.0f, 8.0f, -15.0f);

	// Create the elements we are going to visualize.

	// A simple ball.
	mBallField = new BallScalarField(5, 5, 5, 0, 0, 0, 10, 10, 10, 20, 20, 20);
	mSphereMesh = new MarchingCubesMesh(mBallField, 4, false);

	// A couple of metaballs, which we will animate.
	mMetaballField = new MetaballScalarField(-10, -10, -10, 10, 10, 10, 30, 30, 30);
	mMetaballField->AddMetaball(-7.5, -7.5, -7.5);
	mMetaballField->AddMetaball(-4.5, -4.5, -4.5);
	dX = new float[mMetaballField->Count()];
	dY = new float[mMetaballField->Count()];
	dZ = new float[mMetaballField->Count()];
	for (int i = 0; i < mMetaballField->Count(); i++)
	{
		dX[i] = RandFloat(2.f);
		dY[i] = RandFloat(2.f);
		dZ[i] = RandFloat(2.f);
	}
	mMetaballMesh = new MarchingCubesMesh(mMetaballField, 0.20f, false);
	dIso = 0.025f;
}

MarchingCubesApp::~MarchingCubesApp()
{
	if(mRenderTargetView)
	{
		mRenderTargetView->Release();
		mRenderTargetView = NULL;
	}
	if(mDepthStencilView)
	{
		mDepthStencilView->Release();
		mDepthStencilView = NULL;
	}
	if(mSwapChain)
	{
		mSwapChain->Release();
		mSwapChain = NULL;
	}
	if(mDepthStencilBuffer)
	{
		mDepthStencilBuffer->Release();
		mDepthStencilBuffer = NULL;
	}

	// Restore all default settings.
	if( md3dImmediateContext )
		md3dImmediateContext->ClearState();

	if(md3dImmediateContext)
	{
		md3dImmediateContext->Release();
		md3dImmediateContext = NULL;
	}

	delete mBallField;
	delete mSphereMesh;
	delete mMetaballField;
	delete mMetaballMesh;
	delete[] dX;
	delete[] dY;
	delete[] dZ;

#if defined(DEBUG) || defined(_DEBUG)
	ID3D11Debug * d3dDebug;
	md3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	d3dDebug->Release();
#endif

	if (md3dDevice)
	{
		md3dDevice->Release();
		md3dDevice = NULL;
	}
}

HINSTANCE MarchingCubesApp::AppInst()const
{
	return mhAppInst;
}

HWND MarchingCubesApp::MainWnd()const
{
	return mhMainWnd;
}

float MarchingCubesApp::AspectRatio()const
{
	return static_cast<float>(mClientWidth) / mClientHeight;
}

int MarchingCubesApp::Run()
{
	MSG msg = {0};
 
	mTimer.Reset();

	while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		// Otherwise, do animation/game stuff.
		else
        {	
			mTimer.Tick();

			if( !mAppPaused )
			{
				CalculateFrameStats();
				UpdateScene(mTimer.DeltaTime());	
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
        }
    }

	return (int)msg.wParam;
}

bool MarchingCubesApp::Init()
{
	if(!InitMainWindow())
		return false;

	if(!InitDirect3D())
		return false;

	mSphereMesh->Init(md3dDevice);
	mMetaballMesh->Init(md3dDevice);
	return true;
}
 
void MarchingCubesApp::OnResize()
{
	assert(md3dImmediateContext);
	assert(md3dDevice);
	assert(mSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	if(mRenderTargetView)
	{
		mRenderTargetView->Release();
		mRenderTargetView = NULL;
	}
	if(mDepthStencilView)
	{
		mDepthStencilView->Release();
		mDepthStencilView = NULL;
	}
	if(mDepthStencilBuffer)
	{
		mDepthStencilBuffer->Release();
		mDepthStencilBuffer = NULL;
	}

	// Resize the swap chain and recreate the render target view.

	HRESULT hr = mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
#if defined(DEBUG) | defined(_DEBUG)
	if(FAILED(hr))
	{
		wchar_t msgbuf [512];
		swprintf_s(msgbuf, L"%s (%d) HR=%lX %S\n", __FILE__, (DWORD)__LINE__, hr, L"mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0)");
        OutputDebugString(msgbuf);
	}
#endif

	ID3D11Texture2D* backBuffer;
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	hr = md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView);
	if(backBuffer)
	{
		backBuffer->Release();
		backBuffer = NULL;
	}

	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	
	depthStencilDesc.Width     = mClientWidth;
	depthStencilDesc.Height    = mClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if( mEnable4xMsaa )
	{
		depthStencilDesc.SampleDesc.Count   = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality-1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count   = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags      = 0;

	md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer);
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);


	// Bind the render target view and depth/stencil view to the pipeline.

	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	

	// Set the viewport transform.

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width    = static_cast<float>(mClientWidth);
	mScreenViewport.Height   = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

	// Take into account aspect ratio of the viewport for camera lens.
	float aspect = mScreenViewport.Width / mScreenViewport.Height;
	mCam.SetLens(0.5f*XM_PI, aspect, 0.1f, 100.0f);
}
 
LRESULT MarchingCubesApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE )
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		mClientWidth  = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if( md3dDevice )
		{
			if( wParam == SIZE_MINIMIZED )
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if( wParam == SIZE_MAXIMIZED )
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if( wParam == SIZE_RESTORED )
			{
				
				// Restoring from minimized state?
				if( mMinimized )
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if( mMaximized )
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if( mResizing )
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing  = true;
		mTimer.Stop();
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing  = false;
		mTimer.Start();
		OnResize();
		return 0;
 
	// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


bool MarchingCubesApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = mhAppInst;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, mClientWidth, mClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width  = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"D3DWndClassName", mMainWndCaption.c_str(), 
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0); 
	if( !mhMainWnd )
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

bool MarchingCubesApp::InitDirect3D()
{
	// Create the device and device context.

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
			0,                 // default adapter
			md3dDriverType,
			0,                 // no software device
			createDeviceFlags, 
			0, 0,              // default feature level array
			D3D11_SDK_VERSION,
			&md3dDevice,
			&featureLevel,
			&md3dImmediateContext);

	if( FAILED(hr) )
	{
		hr = D3D11CreateDevice(
			0,                 // default adapter
			D3D_DRIVER_TYPE_REFERENCE,  // Try reference SW driver
			0,                 // no software device
			createDeviceFlags, 
			0, 0,              // default feature level array
			D3D11_SDK_VERSION,
			&md3dDevice,
			&featureLevel,
			&md3dImmediateContext);
		if( FAILED(hr) )
		{
			MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
			return false;
		}
	}

	if( featureLevel != D3D_FEATURE_LEVEL_11_0 )
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	md3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
	assert( m4xMsaaQuality > 0 );

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if( mEnable4xMsaa )
	{
		sd.SampleDesc.Count   = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality-1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count   = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount  = 1;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed     = true;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	      
	IDXGIAdapter* dxgiAdapter = 0;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);

	IDXGIFactory* dxgiFactory = 0;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

	dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain);
	
	if(dxgiDevice)
	{
		dxgiDevice->Release();
		dxgiDevice=NULL;
	}
	if(dxgiFactory)
	{
		dxgiFactory->Release();
		dxgiFactory=NULL;
	}

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
	
	OnResize();

	return true;
}

void MarchingCubesApp::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if( (mTimer.TotalTime() - timeElapsed) >= 1.0f )
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;   
		outs.precision(6);
		outs << mMainWndCaption << L"    "
			 << L"FPS: " << fps << L"    " 
			 << L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(mhMainWnd, outs.str().c_str());
		
		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void MarchingCubesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void MarchingCubesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void MarchingCubesApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = 0.25f*static_cast<float>(x - mLastMousePos.x) * XM_PI / 180.0f;
		float dy = 0.25f*static_cast<float>(y - mLastMousePos.y) * XM_PI / 180.0f;

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void MarchingCubesApp::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
	if( GetAsyncKeyState('W') & 0x8000 )
		mCam.Walk(10.0f*dt);

	if( GetAsyncKeyState('S') & 0x8000 )
		mCam.Walk(-10.0f*dt);

	if( GetAsyncKeyState('A') & 0x8000 )
		mCam.Strafe(-10.0f*dt);

	if( GetAsyncKeyState('D') & 0x8000 )
		mCam.Strafe(10.0f*dt);

	// Move metaballs
	for (int i = 0; i < mMetaballField->Count(); i++)
	{
		float x, y, z;
		mMetaballField->GetMetaball(i, x, y, z);
		x += dX[i] * dt;
		y += dY[i] * dt;
		z += dZ[i] * dt;
		mMetaballField->MoveMetaball(i, x, y, z);
	}

	for (int i = 0; i < mMetaballField->Count(); i++)
	{
		float x, y, z;
		mMetaballField->GetMetaball(i, x, y, z);
		if (x <= mMetaballField->getXOrig() + 2.5 || x >= mMetaballField->getXOrig() + mMetaballField->getWidth() - 2.5)
		{
			dX[i] = -dX[i];
		}
		if (y <= mMetaballField->getYOrig() + 2.5 || y >= mMetaballField->getYOrig() + mMetaballField->getDepth() - 2.5)
		{
			dY[i] = -dY[i];
		}
		if (z <= mMetaballField->getZOrig() + 2.5 || z >= mMetaballField->getZOrig() + mMetaballField->getHeight() - 2.5)
		{
			dZ[i] = -dZ[i];
		}
	}

	float iso = mMetaballMesh->GetIsoLevel();
	iso += dIso * dt;
	if (iso >= 0.50 || iso <= 0.20)
	{
		dIso = -dIso;
	}
	mMetaballMesh->SetIsoLevel(iso);
}


float MarchingCubesApp::RandFloat(float a)
{
	return ((float)rand() / (float)(RAND_MAX)) * a - 0.5f*a;
}


void MarchingCubesApp::DrawScene()
{
	mCam.UpdateViewMatrix();

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	mSphereMesh->Setup(md3dImmediateContext);
	mSphereMesh->Draw(md3dImmediateContext, mCam.View(), mCam.Proj());

	mMetaballMesh->Setup(md3dImmediateContext);
	mMetaballMesh->Draw(md3dImmediateContext, mCam.View(), mCam.Proj());

	mSwapChain->Present(0, 0);
}
