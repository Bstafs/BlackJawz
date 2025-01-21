#pragma once
#include "../pch.h"

ID3D11VertexShader* CreateVertexShader(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11Device* device);
ID3D11PixelShader* CreatePixelShader(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11Device* device);