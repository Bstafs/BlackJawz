#include "../pch.h"
#include "ID3D11Functions.h"

HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

		if (pErrorBlob) pErrorBlob->Release();

		return hr;
	}

	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

ID3DBlob* tempBlob;

ID3D11VertexShader* CreateVertexShader(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11Device* device)
{
	HRESULT hr;
	ID3D11VertexShader* vertexShader = nullptr;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;

	hr = CompileShaderFromFile(szFileName, szEntryPoint, szShaderModel, &pVSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr, L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
	}

	// Create the vertex shader
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &vertexShader);

	tempBlob = pVSBlob;

	if (FAILED(hr))
	{
		pVSBlob->Release();
	}


	return vertexShader;
}

ID3D11PixelShader* CreatePixelShader(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11Device* device)
{
	HRESULT hr;
	ID3D11PixelShader* pixelShader = nullptr;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(szFileName, szEntryPoint, szShaderModel, &pPSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
	}

	// Create the pixel shader
	hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pixelShader);
	pPSBlob->Release();



	return pixelShader;
}

ID3D11InputLayout* CreateInputLayout(D3D11_INPUT_ELEMENT_DESC inputDesc[], UINT numElements, ID3D11Device* device)
{
	HRESULT hr;
	ID3D11InputLayout* inputLayout;

	// Create the input layout
	hr = device->CreateInputLayout(inputDesc, numElements, tempBlob->GetBufferPointer(), tempBlob->GetBufferSize(), &inputLayout);
	tempBlob->Release();

	return inputLayout;
}