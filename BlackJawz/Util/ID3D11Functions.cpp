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
    // Query the underlying resource.
    ComPtr<ID3D11Resource> resource;
    srv->GetResource(&resource);

    // Query for the texture interface.
    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = resource.As(&texture);
    if (FAILED(hr))
    {
        // Resource is not a Texture2D.
        return {};
    }

    // Get the texture description.
    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);

    // For this example, we only support 2D textures.
    if (desc.ArraySize != 1)
    {
        return {};
    }

    // Create a staging texture to read back data.
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> stagingTexture;
    hr = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
    if (FAILED(hr))
    {
        return {};
    }

    // Copy the texture to the staging resource.
    context->CopyResource(stagingTexture.Get(), texture.Get());

    // Map the staging texture to access its data.
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    hr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr))
    {
        return {};
    }

    // Create a minimal DDS header.
    DDS_PIXELFORMAT ddspf = {};
    ddspf.dwSize = sizeof(DDS_PIXELFORMAT);

    // In this example we support only DXGI_FORMAT_R8G8B8A8_UNORM.
    if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
    {
        // Flags: DDPF_RGB (0x40) | DDPF_ALPHAPIXELS (0x1) => 0x41.
        ddspf.dwFlags = 0x41;
        ddspf.dwFourCC = 0; // No FourCC for uncompressed formats.
        ddspf.dwRGBBitCount = 32;
        ddspf.dwRBitMask = 0x00FF0000;
        ddspf.dwGBitMask = 0x0000FF00;
        ddspf.dwBBitMask = 0x000000FF;
        ddspf.dwABitMask = 0xFF000000;
    }
    else
    {
        // Format not supported in this sample.
        context->Unmap(stagingTexture.Get(), 0);
        return {};
    }

    DDS_HEADER header = {};
    header.dwSize = sizeof(DDS_HEADER);
    // Flags: DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE
    header.dwFlags = 0x00021007;
    header.dwHeight = desc.Height;
    header.dwWidth = desc.Width;
    // For uncompressed data, pitch = row size in bytes.
    header.dwPitchOrLinearSize = mapped.RowPitch;
    header.dwDepth = 0;
    header.dwMipMapCount = desc.MipLevels;
    // Zero out reserved fields.
    memset(header.dwReserved1, 0, sizeof(header.dwReserved1));
    header.ddspf = ddspf;
    // Caps: DDSCAPS_TEXTURE
    header.dwCaps = 0x1000;
    header.dwCaps2 = 0;
    header.dwCaps3 = 0;
    header.dwCaps4 = 0;
    header.dwReserved2 = 0;

    // Calculate the size of the pixel data for the base level.
    // (If you wanted to support mipmaps, you would loop through each mip level.)
    size_t dataSize = mapped.RowPitch * desc.Height;

    // Allocate a vector to hold the DDS file data.
    std::vector<uint8_t> ddsBuffer;
    // Total size: 4 bytes for the magic, plus the header, plus the pixel data.
    size_t totalSize = sizeof(uint32_t) + sizeof(DDS_HEADER) + dataSize;
    ddsBuffer.resize(totalSize);

    // Pointer to write into the vector.
    uint8_t* ptr = ddsBuffer.data();

    // Write the DDS magic number.
    *reinterpret_cast<uint32_t*>(ptr) = DDS_MAGIC;
    ptr += sizeof(uint32_t);

    // Write the DDS header.
    memcpy(ptr, &header, sizeof(DDS_HEADER));
    ptr += sizeof(DDS_HEADER);

    // Copy the pixel data from the mapped resource.
    // Note: This simple example assumes that the texture is tightly packed.
    memcpy(ptr, mapped.pData, dataSize);

    // Unmap the staging texture.
    context->Unmap(stagingTexture.Get(), 0);

    return ddsBuffer;
}
