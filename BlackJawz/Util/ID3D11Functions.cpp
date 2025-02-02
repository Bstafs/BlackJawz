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

std::vector<uint8_t> ExtractDDSData(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
    // Get the texture resource from SRV
    ComPtr<ID3D11Resource> resource;
    srv->GetResource(&resource);

    // Cast to Texture2D
    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = resource.As(&texture);
    if (FAILED(hr))
        return {};

    // Get texture description
    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);

    // Create a staging texture
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> stagingTexture;
    hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
    if (FAILED(hr))
        return {};

    // Copy resource to staging texture
    context->CopyResource(stagingTexture.Get(), texture.Get());

    // Map staging texture
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    hr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr))
        return {};

    // Compute size
    size_t dataSize = mapped.RowPitch * desc.Height;

    // Allocate buffer for raw pixel data
    std::vector<uint8_t> ddsData(dataSize);

    uint8_t* dest = ddsData.data();
    uint8_t* src = static_cast<uint8_t*>(mapped.pData);

    for (UINT row = 0; row < desc.Height; ++row)
    {
        memcpy(dest, src, mapped.RowPitch);
        dest += mapped.RowPitch;
        src += mapped.RowPitch;
    }

    context->Unmap(stagingTexture.Get(), 0);

    return ddsData;
}

void LogDebug(const std::string& message)
{
    OutputDebugStringA(message.c_str());
}