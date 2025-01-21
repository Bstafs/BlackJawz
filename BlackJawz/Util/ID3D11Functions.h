#pragma once
#include "../pch.h"

HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
ID3D11VertexShader* CreateVertexShader(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11Device* device);
ID3D11PixelShader* CreatePixelShader(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11Device* device);
ID3D11InputLayout* CreateInputLayout(D3D11_INPUT_ELEMENT_DESC inputDesc[], UINT numElements, ID3D11Device* device);