#include "Globals.h"



//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	
    ID3DBlob* pErrorBlob = NULL;
    hr =  //D3DX11CompileFromFile
		D3DCompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel,  dwShaderFlags, /*0,*/ NULL, ppBlobOut, &pErrorBlob /*, NULL*/ );
    
    if( FAILED(hr) )
    {
        if (pErrorBlob != NULL)
        {
            auto error_string = (char*)pErrorBlob->GetBufferPointer();
            OutputDebugStringA(error_string);
        }
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}