#include "Rendering.h"

BlackJawz::Rendering::Render::Render()
{
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
}

BlackJawz::Rendering::Render::~Render()
{

}

HRESULT BlackJawz::Rendering::Render::InitDeviceAndSwapChain()
{
	HRESULT hr = S_OK;
	UINT createDeviceFlags = 0;

	// Enable debug layer in debug builds
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Supported driver types
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	// Supported feature levels
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// Swap chain description
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = renderWidth;
	sd.BufferDesc.Height = renderHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = BlackJawz::Application::Application::GetWindow();
	sd.SampleDesc.Count = mSampleCount; // Anti-aliasing sample count
	sd.SampleDesc.Quality = 0; // Quality level
	sd.Windowed = TRUE; // Windowed mode

	// Attempt to create device and swap chain with different driver types
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,                            // Adapter (nullptr = use default)
			driverTypes[driverTypeIndex],       // Driver type
			nullptr,                            // Software rasterizer (unused)
			createDeviceFlags,                  // Device creation flags
			featureLevels,                      // Feature levels
			numFeatureLevels,                   // Number of feature levels
			D3D11_SDK_VERSION,                  // SDK version
			&sd,                                // Swap chain description
			pSwapChain.GetAddressOf(),          // Swap chain pointer
			pID3D11Device.GetAddressOf(),       // Device pointer
			nullptr,                            // Feature level achieved
			pImmediateContext.GetAddressOf()    // Device context pointer
		);

		if (SUCCEEDED(hr))
		{
			break;
		}
	}

	if (FAILED(hr))
	{
		return hr; // Return failure if no driver type works
	}

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitRenderTargetViews()
{
	if (FAILED(InitBackBuffer()))
	{
		return E_FAIL;
	}

	if (FAILED(InitLightingView()))
	{
		return E_FAIL;
	}

	if (FAILED(InitQuadView()))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT BlackJawz::Rendering::Render::InitBackBuffer()
{
	HRESULT hr = S_OK;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = pSwapChain.Get()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	if (FAILED(hr))
		return hr;

	hr = pID3D11Device.Get()->CreateRenderTargetView(pBackBuffer, nullptr, pRenderTargetView.GetAddressOf());
	pBackBuffer->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = renderWidth;
	textureDesc.Height = renderHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = mSampleCount;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, pRenderTexture.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	// Create a render target view for the texture
	hr = pID3D11Device.Get()->CreateRenderTargetView(pRenderTexture.Get(), nullptr, pRenderTargetTextureView.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	// Create a shader resource view for the texture
	hr = pID3D11Device.Get()->CreateShaderResourceView(pRenderTexture.Get(), nullptr, pShaderResourceView.GetAddressOf());
	if (FAILED(hr)) {
		return hr;
	}

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitLightingView()
{
	HRESULT hr = S_OK;

	// Create a render target texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = renderWidth;
	textureDesc.Height = renderHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = mSampleCount;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, g_pGbufferTargetLightingTextures.GetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	hr = pID3D11Device.Get()->CreateRenderTargetView(g_pGbufferTargetLightingTextures.Get(), &renderTargetViewDesc, g_pGbufferRenderLightingTargetView.GetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = pID3D11Device.Get()->CreateShaderResourceView(g_pGbufferTargetLightingTextures.Get(), &shaderResourceViewDesc, g_pGbufferShaderResourceLightingView.GetAddressOf());
	if (FAILED(hr))
		return hr;
}

HRESULT BlackJawz::Rendering::Render::InitQuadView()
{
	HRESULT hr = S_OK;

	// Create a render target texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = renderWidth;
	textureDesc.Height = renderHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = mSampleCount;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	 hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, pQuadRenderTargetTexture.GetAddressOf());
	if (FAILED(hr))
		throw std::runtime_error("Failed to create offscreen texture.");

	// Create the RTV
	hr = pID3D11Device.Get()->CreateRenderTargetView(pQuadRenderTargetTexture.Get(), nullptr, pQuadRenderTargetView.GetAddressOf());
	if (FAILED(hr))
		throw std::runtime_error("Failed to create offscreen RTV.");

	// Create the SRV
	hr = pID3D11Device.Get()->CreateShaderResourceView(pQuadRenderTargetTexture.Get(), nullptr, pQuadRenderShaderResourceView.GetAddressOf());
	if (FAILED(hr))
		throw std::runtime_error("Failed to create offscreen SRV.");
}

void BlackJawz::Rendering::Render::ResizeRenderTarget(int width, int height)
{
	if (width == renderWidth && height == renderHeight)
	{
		return;
	}

	// Update stored render target size FIRST
	renderWidth = width;
	renderHeight = height;

	// Reset old resources
	pRenderTargetTextureView.Reset();
	pRenderTexture.Reset();
	pShaderResourceView.Reset();

	g_pGbufferTargetLightingTextures.Reset();
	g_pGbufferRenderLightingTargetView.Reset();
	g_pGbufferShaderResourceLightingView.Reset();

	pQuadRenderTargetTexture.Reset();
	pQuadRenderTargetView.Reset();
	pQuadRenderShaderResourceView.Reset();

	// Recreate the main render target texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = renderWidth;
	textureDesc.Height = renderHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = mSampleCount;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, pRenderTexture.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to resize render target texture.");
	}

	// Recreate the render target view
	hr = pID3D11Device.Get()->CreateRenderTargetView(pRenderTexture.Get(), nullptr, pRenderTargetTextureView.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create render target view.");
	}

	// Recreate the shader resource view
	hr = pID3D11Device.Get()->CreateShaderResourceView(pRenderTexture.Get(), nullptr, pShaderResourceView.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create shader resource view.");
	}

	// Lighting Pass
	 hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, g_pGbufferTargetLightingTextures.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to resize render target texture.");
	}

	// Recreate the render target view
	hr = pID3D11Device.Get()->CreateRenderTargetView(g_pGbufferTargetLightingTextures.Get(), nullptr, g_pGbufferRenderLightingTargetView.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create render target view.");
	}

	// Recreate the shader resource view
	hr = pID3D11Device.Get()->CreateShaderResourceView(g_pGbufferTargetLightingTextures.Get(), nullptr, g_pGbufferShaderResourceLightingView.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create shader resource view.");
	}

	// Quad Pass
	hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, pQuadRenderTargetTexture.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to resize render target texture.");
	}

	// Recreate the render target view
	hr = pID3D11Device.Get()->CreateRenderTargetView(pQuadRenderTargetTexture.Get(), nullptr, pQuadRenderTargetView.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create render target view.");
	}

	// Recreate the shader resource view
	hr = pID3D11Device.Get()->CreateShaderResourceView(pQuadRenderTargetTexture.Get(), nullptr, pQuadRenderShaderResourceView.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create shader resource view.");
	}

	ResizeDepthStencilBuffer();
	ResizeGBuffer();
}

void BlackJawz::Rendering::Render::ResizeDepthStencilBuffer()
{
	pDepthStencilView.Reset();
	pDepthStencilBuffer.Reset();

	// Depth buffer must match the current render target size
	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = renderWidth;
	depthStencilDesc.Height = renderHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = mSampleCount;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HRESULT hr = pID3D11Device.Get()->CreateTexture2D(&depthStencilDesc, nullptr, pDepthStencilBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create depth-stencil buffer.");
	}

	// Create depth-stencil view
	hr = pID3D11Device.Get()->CreateDepthStencilView(pDepthStencilBuffer.Get(), nullptr, pDepthStencilView.GetAddressOf());
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create depth-stencil view.");
	}
}


void BlackJawz::Rendering::Render::ResizeGBuffer()
{
	for (int i = 0; i < 4; i++)
	{
		gBufferTextures[i].Reset();
		gBufferRTVs[i].Reset();
		gBufferSRVs[i].Reset();
	}

	DXGI_FORMAT formats[] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,      // Albedo
		DXGI_FORMAT_R16G16B16A16_FLOAT,  // Normal
		DXGI_FORMAT_R16G16B16A16_FLOAT,  // Position
		DXGI_FORMAT_R8G8B8A8_UNORM       // Specular 
	};

	HRESULT hr = S_OK;

	for (int i = 0; i < 4; i++)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = renderWidth;
		textureDesc.Height = renderHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = formats[i];
		textureDesc.SampleDesc.Count = mSampleCount;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// Recreate Texture
		hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, gBufferTextures[i].GetAddressOf());
		if (FAILED(hr))
			throw std::runtime_error("Failed to resize GBuffer texture.");

		// Recreate Render Target View (RTV)
		hr = pID3D11Device.Get()->CreateRenderTargetView(gBufferTextures[i].Get(), nullptr, gBufferRTVs[i].GetAddressOf());
		if (FAILED(hr))
			throw std::runtime_error("Failed to create GBuffer RTV.");

		// Recreate Shader Resource View (SRV)
		hr = pID3D11Device.Get()->CreateShaderResourceView(gBufferTextures[i].Get(), nullptr, gBufferSRVs[i].GetAddressOf());
		if (FAILED(hr))
			throw std::runtime_error("Failed to create GBuffer SRV.");
	}
}

HRESULT BlackJawz::Rendering::Render::InitViewPort()
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = renderWidth;
	vp.Height = renderHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pImmediateContext.Get()->RSSetViewports(1, &vp);

	return S_OK;
}

HRESULT BlackJawz::Rendering::Render::InitShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	// Compile the vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
	hr = CompileShaderFromFile(L"../BlackJawz/Rendering/Shaders/shader.hlsl", "MainVS", "vs_5_0", &vsBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to compile vertex shader.\n");
		return hr;
	}

	// Compile the pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	hr = CompileShaderFromFile(L"../BlackJawz/Rendering/Shaders/shader.hlsl", "PS", "ps_5_0", &psBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to compile pixel shader.\n");
		return hr;
	}

	// Create the vertex shader
	hr = pID3D11Device.Get()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create vertex shader.\n");
		return hr;
	}

	// Create the pixel shader
	hr = pID3D11Device.Get()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pPixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create pixel shader.\n");
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	hr = pID3D11Device.Get()->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), pInputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create input layout.\n");
		return hr;
	}

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitGBufferShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	// Compile the vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
	hr = CompileShaderFromFile(L"../BlackJawz/Rendering/Shaders/GBufferVertexShader.hlsl", "VS", "vs_5_0", &vsBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to compile vertex shader.\n");
		return hr;
	}

	// Compile the pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	hr = CompileShaderFromFile(L"../BlackJawz/Rendering/Shaders/GBufferPixelShader.hlsl", "PS", "ps_5_0", &psBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to compile pixel shader.\n");
		return hr;
	}

	// Create the vertex shader
	hr = pID3D11Device.Get()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, pGBufferVertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create vertex shader.\n");
		return hr;
	}

	// Create the pixel shader
	hr = pID3D11Device.Get()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pGBufferPixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create pixel shader.\n");
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	hr = pID3D11Device.Get()->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), pGBufferInputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create input layout.\n");
		return hr;
	}

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitDeferredLightingShaders()
{
	HRESULT hr = S_OK;

	// Compile the vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
	hr = CompileShaderFromFile(L"../BlackJawz/Rendering/Shaders/DeferredLightingPixelShader.hlsl", "VS", "vs_5_0", &vsBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to compile vertex shader.\n");
		return hr;
	}

	// Compile the pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	hr = CompileShaderFromFile(L"../BlackJawz/Rendering/Shaders/DeferredLightingPixelShader.hlsl", "PS", "ps_5_0", &psBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to compile pixel shader.\n");
		return hr;
	}

	// Create the vertex shader
	hr = pID3D11Device.Get()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, pDeferredLightingVertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create vertex shader.\n");
		return hr;
	}

	// Create the pixel shader
	hr = pID3D11Device.Get()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pDeferredLightingPixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create pixel shader.\n");
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	hr = pID3D11Device.Get()->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), pDeferredLightingInputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create input layout.\n");
		return hr;
	}

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitPostProcessingShaders()
{
	HRESULT hr = S_OK;

	// Compile the vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
	hr = CompileShaderFromFile(L"../BlackJawz/Rendering/Shaders/PostProcessingShader.hlsl", "VS", "vs_5_0", &vsBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to compile vertex shader.\n");
		return hr;
	}

	// Compile the pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
	hr = CompileShaderFromFile(L"../BlackJawz/Rendering/Shaders/PostProcessingShader.hlsl", "PS", "ps_5_0", &psBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to compile pixel shader.\n");
		return hr;
	}

	// Create the vertex shader
	hr = pID3D11Device.Get()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, pPostProcessingVertexShader.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create vertex shader.\n");
		return hr;
	}

	// Create the pixel shader
	hr = pID3D11Device.Get()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, pPostProcessingPixelShader.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create pixel shader.\n");
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	hr = pID3D11Device.Get()->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), pPostProcessingInputLayout.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to create input layout.\n");
		return hr;
	}

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitSamplerState()
{
	HRESULT hr = S_OK;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pID3D11Device.Get()->CreateSamplerState(&sampDesc, pSamplerLinear.GetAddressOf());

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitDepthStencil()
{
	// Depth Stencil 

	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = renderWidth;
	depthStencilDesc.Height = renderHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = mSampleCount;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	pID3D11Device.Get()->CreateTexture2D(&depthStencilDesc, nullptr, pDepthStencilBuffer.GetAddressOf());
	pID3D11Device.Get()->CreateDepthStencilView(pDepthStencilBuffer.Get(), nullptr, pDepthStencilView.GetAddressOf());

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitRasterizer()
{
	// Rasterizer
	HRESULT hr = S_OK;

	D3D11_DEPTH_STENCIL_DESC stencilDesc = {};
	stencilDesc.DepthEnable = true;
	stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	stencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	pID3D11Device.Get()->CreateDepthStencilState(&stencilDesc, DSLessEqual.GetAddressOf());

	D3D11_RASTERIZER_DESC cmdesc = {};
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;
	cmdesc.FrontCounterClockwise = false;

	hr = pID3D11Device.Get()->CreateRasterizerState(&cmdesc, CWcullMode.GetAddressOf());

	pImmediateContext.Get()->OMSetDepthStencilState(DSLessEqual.Get(), 1);
	pImmediateContext.Get()->RSSetState(CWcullMode.Get());
	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitImGui()
{
	HRESULT hr = S_OK;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_DockingEnable;

	ImGui_ImplWin32_Init(BlackJawz::Application::Application::GetWindow());
	ImGui_ImplDX11_Init(pID3D11Device.Get(), pImmediateContext.Get());
	ImGui::StyleColorsDark();

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitConstantBuffer()
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC bufferDescTransform = {};
	bufferDescTransform.Usage = D3D11_USAGE_DEFAULT;
	bufferDescTransform.ByteWidth = sizeof(TransformBuffer);
	bufferDescTransform.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescTransform.CPUAccessFlags = 0;
	bufferDescTransform.MiscFlags = 0;
	bufferDescTransform.StructureByteStride = 0;

	hr = pID3D11Device.Get()->CreateBuffer(&bufferDescTransform, nullptr, pTransformBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create constant buffer.\n");
		return hr;
	}

	D3D11_BUFFER_DESC bufferDescLights = {};
	bufferDescLights.Usage = D3D11_USAGE_DEFAULT;
	bufferDescLights.ByteWidth = sizeof(LightsBuffer);
	bufferDescLights.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescLights.CPUAccessFlags = 0;
	bufferDescLights.MiscFlags = 0;
	bufferDescLights.StructureByteStride = 0;

	hr = pID3D11Device.Get()->CreateBuffer(&bufferDescLights, nullptr, pLightsBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create constant buffer.\n");
		return hr;
	}

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitCube()
{
	HRESULT hr = S_OK;

	// Create vertex buffer
	Vertex vertices[] =
	{
		{{-1.0f, 1.0f, -1.0f }, {-1.0f, 1.0f, -1.0f  }, {1.0f, 0.0f} },
		{{1.0f, 1.0f, -1.0f  }, {1.0f, 1.0f, -1.0f   }, {0.0f, 0.0f} },
		{{1.0f, 1.0f, 1.0f   }, {1.0f, 1.0f, 1.0f    }, {0.0f, 1.0f} },
		{{-1.0f, 1.0f, 1.0f  }, {-1.0f, 1.0f, 1.0f	 }, {1.0f, 1.0f} },

		{{-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f }, {0.0f, 0.0f} },
		{{1.0f, -1.0f, -1.0f }, {1.0f, -1.0f, -1.0f  }, {1.0f, 0.0f} },
		{{1.0f, -1.0f, 1.0f  }, {1.0f, -1.0f, 1.0f   }, {1.0f, 1.0f} },
		{{-1.0f, -1.0f, 1.0f }, {-1.0f, -1.0f, 1.0f  }, {0.0f, 1.0f} },

		{{-1.0f, -1.0f, 1.0f }, {-1.0f, -1.0f, 1.0f  }, {0.0f, 1.0f} },
		{{-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f }, {1.0f, 1.0f} },
		{{-1.0f, 1.0f, -1.0f }, {-1.0f, 1.0f, -1.0f  }, {1.0f, 0.0f} },
		{{-1.0f, 1.0f, 1.0f  }, {-1.0f, 1.0f, 1.0f   }, {0.0f, 0.0f} },

		{{1.0f, -1.0f, 1.0f  }, {1.0f, -1.0f, 1.0f   }, {1.0f, 1.0f} },
		{{1.0f, -1.0f, -1.0f }, {1.0f, -1.0f, -1.0f  }, {0.0f, 1.0f} },
		{{1.0f, 1.0f, -1.0f  }, {1.0f, 1.0f, -1.0f   }, {0.0f, 0.0f} },
		{{1.0f, 1.0f, 1.0f   }, {1.0f, 1.0f, 1.0f    }, {1.0f, 0.0f} },

		{{-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f }, {0.0f, 1.0f} },
		{{1.0f, -1.0f, -1.0f }, {1.0f, -1.0f, -1.0f  }, {1.0f, 1.0f} },
		{{1.0f, 1.0f, -1.0f  }, {1.0f, 1.0f, -1.0f   }, {1.0f, 0.0f} },
		{{-1.0f, 1.0f, -1.0f }, {-1.0f, 1.0f, -1.0f  }, {0.0f, 0.0f} },

		{{-1.0f, -1.0f, 1.0f }, {-1.0f, -1.0f, 1.0f  }, {1.0f, 1.0f} },
		{{1.0f, -1.0f, 1.0f  }, {1.0f, -1.0f, 1.0f   }, {0.0f, 1.0f} },
		{{1.0f, 1.0f, 1.0f   }, {1.0f, 1.0f, 1.0f    }, {0.0f, 0.0f} },
		{{-1.0f, 1.0f, 1.0f  }, {-1.0f, 1.0f, 1.0f   }, {1.0f, 0.0f} },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = pID3D11Device.Get()->CreateBuffer(&bd, &InitData, pCubeVertexBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	// Create index buffer
	uint32_t indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(uint32_t) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pID3D11Device.Get()->CreateBuffer(&bd, &InitData, pCubeIndexBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	return hr; // Return S_OK if everything succeeded
}

HRESULT BlackJawz::Rendering::Render::InitSphere()
{
	HRESULT hr = S_OK;

	float radius = 1.0f;
	float numSubdivisions = 32.0f;

	const float pi = XM_PI;
	const float twoPi = 2.0f * pi;

	// Generate vertices
	for (int i = 0; i <= numSubdivisions; ++i)
	{
		// Golden Ratio
		float phi = pi * static_cast<float>(i) / numSubdivisions;

		for (int j = 0; j <= numSubdivisions; ++j)
		{
			float theta = twoPi * static_cast<float>(j) / numSubdivisions;

			Vertex vertex;

			vertex.Position.x = radius * sin(phi) * cos(theta);
			vertex.Position.y = radius * cos(phi);
			vertex.Position.z = radius * sin(phi) * sin(theta);

			vertex.Normal.x = vertex.Position.x / radius;
			vertex.Normal.y = vertex.Position.y / radius;
			vertex.Normal.z = vertex.Position.z / radius;

			vertex.TexC.x = static_cast<float>(j) / numSubdivisions;
			vertex.TexC.y = static_cast<float>(i) / numSubdivisions;

			sphereVertices.push_back(vertex);
		}
	}

	// Generate indices
	for (int i = 0; i < numSubdivisions; ++i) {
		for (int j = 0; j < numSubdivisions; ++j) {
			int v0 = i * (numSubdivisions + 1) + j;
			int v1 = v0 + 1;
			int v2 = (i + 1) * (numSubdivisions + 1) + j;
			int v3 = v2 + 1;

			sphereIndices.push_back(v0);
			sphereIndices.push_back(v1);
			sphereIndices.push_back(v2);

			sphereIndices.push_back(v1);
			sphereIndices.push_back(v3);
			sphereIndices.push_back(v2);
		}
	}

	D3D11_BUFFER_DESC vbDesc, ibDesc;
	D3D11_SUBRESOURCE_DATA vbData, ibData;

	// Vertex Buffer
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(Vertex) * sphereVertices.size();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbData.pSysMem = sphereVertices.data();
	pID3D11Device.Get()->CreateBuffer(&vbDesc, &vbData, pSphereVertexBuffer.GetAddressOf());

	// Index Buffer
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(uint32_t) * sphereIndices.size();
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibData.pSysMem = sphereIndices.data();
	pID3D11Device.Get()->CreateBuffer(&ibDesc, &ibData, pSphereIndexBuffer.GetAddressOf());

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitPlane()
{
	HRESULT hr = S_OK;

	float width = 10.0f;
	float depth = 10.0f;
	uint32_t m = 10;  // Rows
	uint32_t n = 10;  // Columns

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f;  // Each triangle gets full UV range
	float dv = 1.0f;

	uint32_t k = 0;

	// Iterate over the grid and generate vertices for each triangle separately
	for (uint32_t i = 0; i < m - 1; ++i)
	{
		for (uint32_t j = 0; j < n - 1; ++j)
		{
			// Define four corners of a quad
			XMFLOAT3 p0(-halfWidth + j * dx, 0.0f, halfDepth - i * dz);
			XMFLOAT3 p1(-halfWidth + (j + 1) * dx, 0.0f, halfDepth - i * dz);
			XMFLOAT3 p2(-halfWidth + j * dx, 0.0f, halfDepth - (i + 1) * dz);
			XMFLOAT3 p3(-halfWidth + (j + 1) * dx, 0.0f, halfDepth - (i + 1) * dz);

			// Triangle 1
			gridVertices.push_back({ p0, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) });
			gridVertices.push_back({ p1, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) });
			gridVertices.push_back({ p2, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) });

			// Triangle 2
			gridVertices.push_back({ p2, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) });
			gridVertices.push_back({ p1, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) });
			gridVertices.push_back({ p3, XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) });

			gridIndices.push_back(k);
			gridIndices.push_back(k + 1);
			gridIndices.push_back(k + 2);
			gridIndices.push_back(k + 3);
			gridIndices.push_back(k + 4);
			gridIndices.push_back(k + 5);

			k += 6; // Move to the next set of triangles
		}
	}

	// Create Vertex Buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * gridVertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = gridVertices.data();
	hr = pID3D11Device->CreateBuffer(&bd, &initData, pPlaneVertexBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	// Create Index Buffer
	bd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * gridIndices.size());
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	initData.pSysMem = gridIndices.data();
	hr = pID3D11Device->CreateBuffer(&bd, &initData, pPlaneIndexBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	return hr; // Return S_OK if everything succeeded
}

HRESULT BlackJawz::Rendering::Render::InitGBuffer()
{
	HRESULT hr = S_OK;

	// Define formats for each G-Buffer texture.
	// For the depth texture, use a typeless format so it can be bound as both a depth-stencil and a shader resource.
	DXGI_FORMAT formats[] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,      // Albedo
		DXGI_FORMAT_R16G16B16A16_FLOAT,  // Normal
		DXGI_FORMAT_R16G16B16A16_FLOAT,  // Position
		DXGI_FORMAT_R8G8B8A8_UNORM        // Specular 
	};

	for (int i = 0; i < 4; i++) // First four textures: Albedo, Normal, Position, Specular
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = renderWidth;
		textureDesc.Height = renderHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = formats[i];
		textureDesc.SampleDesc.Count = mSampleCount;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// Create Texture
		hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, gBufferTextures[i].GetAddressOf());
		if (FAILED(hr))
			return hr;

		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create Render Target View (RTV)
		hr = pID3D11Device.Get()->CreateRenderTargetView(gBufferTextures[i].Get(), &renderTargetViewDesc, gBufferRTVs[i].GetAddressOf());
		if (FAILED(hr))
			return hr;

		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create Shader Resource View (SRV)
		hr = pID3D11Device.Get()->CreateShaderResourceView(gBufferTextures[i].Get(), &shaderResourceViewDesc, gBufferSRVs[i].GetAddressOf());
		if (FAILED(hr))
			return hr;
	}

	//Blend state setup
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pID3D11Device.Get()->CreateBlendState(&blendDesc, &blendState);

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitDeferredQuad()
{
	HRESULT hr = S_OK;

	VertexQuad vertices[4] =
	{
	{ {-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f} }, // Top-left
	{ { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f} }, // Top-right
	{ {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} }, // Bottom-left
	{ { 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} }  // Bottom-right
	};


	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VertexQuad) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = pID3D11Device.Get()->CreateBuffer(&bd, &InitData, pDeferredQuadVB.GetAddressOf());
	if (FAILED(hr))
		return hr;

	WORD indices[] =
	{
		0, 1, 2,  // First Triangle (Top-left, Top-right, Bottom-left)
		1, 3, 2   // Second Triangle (Top-right, Bottom-right, Bottom-left)
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pID3D11Device.Get()->CreateBuffer(&bd, &InitData, pDeferredQuadIB.GetAddressOf());
	if (FAILED(hr))
		return hr;

	return hr;
}

HRESULT BlackJawz::Rendering::Render::Initialise()
{
	if (FAILED(InitDeviceAndSwapChain()))
	{
		return E_FAIL;
	}

	if (FAILED(InitRenderTargetViews()))
	{
		return E_FAIL;
	}

	if (FAILED(InitViewPort()))
	{
		return E_FAIL;
	}

	if (FAILED(InitShadersAndInputLayout()))
	{
		return E_FAIL;
	}

	if (FAILED(InitGBufferShadersAndInputLayout()))
	{
		return E_FAIL;
	}

	if (FAILED(InitDeferredLightingShaders()))
	{
		return E_FAIL;
	}

	if (FAILED(InitPostProcessingShaders()))
	{
		return E_FAIL;
	}

	if (FAILED(InitSamplerState()))
	{
		return E_FAIL;
	}

	if (FAILED(InitDepthStencil()))
	{
		return E_FAIL;
	}

	if (FAILED(InitRasterizer()))
	{
		return E_FAIL;
	}

	if (FAILED(InitImGui()))
	{
		return E_FAIL;
	}

	if (FAILED(InitConstantBuffer()))
	{
		return E_FAIL;
	}

	if (FAILED(InitCube()))
	{
		return E_FAIL;
	}

	if (FAILED(InitSphere()))
	{
		return E_FAIL;
	}

	if (FAILED(InitPlane()))
	{
		return E_FAIL;
	}

	if (FAILED(InitGBuffer()))
	{
		return E_FAIL;
	}

	if (FAILED(InitDeferredQuad()))
	{
		return E_FAIL;
	}

	return S_OK;
}

BlackJawz::Component::Geometry BlackJawz::Rendering::Render::CreateCubeGeometry()
{
	BlackJawz::Component::Geometry cubeGeometry;
	cubeGeometry.pIndexBuffer = pCubeIndexBuffer;
	cubeGeometry.pVertexBuffer = pCubeVertexBuffer;
	cubeGeometry.IndicesCount = 36;
	cubeGeometry.vertexBufferOffset = 0;
	cubeGeometry.vertexBufferStride = sizeof(Vertex);
	return cubeGeometry;
}

BlackJawz::Component::Geometry BlackJawz::Rendering::Render::CreateSphereGeometry()
{
	BlackJawz::Component::Geometry sphereGeometry;
	sphereGeometry.pIndexBuffer = pSphereIndexBuffer;
	sphereGeometry.pVertexBuffer = pSphereVertexBuffer;
	sphereGeometry.IndicesCount = sphereIndices.size();
	sphereGeometry.vertexBufferOffset = 0;
	sphereGeometry.vertexBufferStride = sizeof(Vertex);
	return sphereGeometry;
}

BlackJawz::Component::Geometry BlackJawz::Rendering::Render::CreatePlaneGeometry()
{
	BlackJawz::Component::Geometry planeGeometry;
	planeGeometry.pIndexBuffer = pPlaneIndexBuffer;
	planeGeometry.pVertexBuffer = pPlaneVertexBuffer;
	planeGeometry.IndicesCount = gridIndices.size();
	planeGeometry.vertexBufferOffset = 0;
	planeGeometry.vertexBufferStride = sizeof(Vertex);
	return planeGeometry;
}

void BlackJawz::Rendering::Render::RenderToTexture(BlackJawz::System::TransformSystem& transformSystem,
	BlackJawz::System::AppearanceSystem& appearanceSystem, BlackJawz::System::LightSystem& lightSystem)
{
	// Bind the render target texture
	pImmediateContext.Get()->OMSetRenderTargets(1, pRenderTargetTextureView.GetAddressOf(), nullptr);

	// Set up the viewport for rendering to the texture
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(renderWidth);
	viewport.Height = static_cast<float>(renderHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	pImmediateContext.Get()->RSSetViewports(1, &viewport);

	// Clear the render target
	SetBackGroundColour(0.67f, 0.74f, 1.0f, 1.0f);
	pImmediateContext.Get()->ClearRenderTargetView(pRenderTargetTextureView.Get(), ClearColor);
	pImmediateContext.Get()->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render 3D scene
	Draw(transformSystem, appearanceSystem, lightSystem);

	// Restore the default render target after rendering to texture
	pImmediateContext.Get()->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);
}

void BlackJawz::Rendering::Render::Update()
{

}

void BlackJawz::Rendering::Render::BeginFrame()
{
	// Clear the back buffer
	SetBackGroundColour(0.0f, 0.0f, 0.0f, 1.0f);
	pImmediateContext.Get()->ClearRenderTargetView(pRenderTargetView.Get(), ClearColor);
	pImmediateContext.Get()->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	pImmediateContext.Get()->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());
}

void BlackJawz::Rendering::Render::BeginGBufferPass()
{
	// Clear the G-Buffer textures
	float clearColor[4] = { 0, 0, 0, 0 };
	pImmediateContext.Get()->ClearRenderTargetView(gBufferRTVs[0].Get(), clearColor); // Albedo
	pImmediateContext.Get()->ClearRenderTargetView(gBufferRTVs[1].Get(), clearColor); // Normal
	pImmediateContext.Get()->ClearRenderTargetView(gBufferRTVs[2].Get(), clearColor); // Position
	pImmediateContext.Get()->ClearRenderTargetView(gBufferRTVs[3].Get(), clearColor); // Specular
}

void BlackJawz::Rendering::Render::EndGBufferPass()
{
	// Unbind the G-buffer render targets
	ID3D11RenderTargetView* nullRTV[4] = { nullptr, nullptr, nullptr,nullptr };
	pImmediateContext.Get()->OMSetRenderTargets(4, nullRTV, nullptr);

	// Unbind shader resource views
	ID3D11ShaderResourceView* nullSRVs[4] = { nullptr, nullptr, nullptr, nullptr };
	pImmediateContext.Get()->PSSetShaderResources(0, 4, nullSRVs);
}

//void BlackJawz::Rendering::Render::Draw(BlackJawz::System::TransformSystem& transformSystem,
//	BlackJawz::System::AppearanceSystem& appearanceSystem, BlackJawz::System::LightSystem& lightSystem)
//{
//	transformSystem.Update();
//	lightSystem.Update();
//
//	TransformBuffer tcb = {};
//	LightsBuffer lcb = {};
//
//	// Load View and Projection Matrices
//	XMMATRIX view = XMLoadFloat4x4(&viewMatrix);
//	XMMATRIX projection = XMLoadFloat4x4(&projectionMatrix);
//
//	tcb.View = XMMatrixTranspose(view);
//	tcb.Projection = XMMatrixTranspose(projection);
//
//	// Set shaders and constant buffers
//	pImmediateContext.Get()->IASetInputLayout(pInputLayout.Get());
//	pImmediateContext.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	pImmediateContext.Get()->VSSetShader(pVertexShader.Get(), nullptr, 0);
//	pImmediateContext.Get()->VSSetConstantBuffers(0, 1, pTransformBuffer.GetAddressOf());
//	pImmediateContext.Get()->VSSetConstantBuffers(1, 1, pLightsBuffer.GetAddressOf());
//	pImmediateContext.Get()->PSSetShader(pPixelShader.Get(), nullptr, 0);
//	pImmediateContext.Get()->PSSetConstantBuffers(0, 1, pTransformBuffer.GetAddressOf());
//	pImmediateContext.Get()->PSSetConstantBuffers(1, 1, pLightsBuffer.GetAddressOf());
//	pImmediateContext.Get()->PSSetSamplers(0, 1, pSamplerLinear.GetAddressOf());
//
//	// Global Camera Position
//	lcb.CameraPosition = cameraPosition;
//	pImmediateContext.Get()->UpdateSubresource(pLightsBuffer.Get(), 0, nullptr, &lcb, 0, 0);
//
//	// Process all lights before rendering
//	if (!lightSystem.GetEntities().empty())
//	{
//		// Clear light buffer
//		ZeroMemory(lcb.lights, sizeof(lcb.lights));
//		lcb.numLights = static_cast<int>(lightSystem.GetEntities().size());
//
//		if (!lightSystem.GetEntities().empty())
//		{
//			int lightIndex = 0;
//
//			for (auto lightEntity : lightSystem.GetEntities())
//			{
//				if (lightIndex >= MAX_LIGHTS) break; // Avoid exceeding the limit
//
//				auto& light = lightSystem.GetLight(lightEntity);
//
//				if (transformSystem.HasComponent(lightEntity))
//				{
//					auto& lightTransform = transformSystem.GetTransform(lightEntity);
//
//					XMFLOAT3 lightPosition = lightTransform.GetPosition();
//					lcb.lights[lightIndex].LightPosition = XMFLOAT4(lightPosition.x, lightPosition.y, lightPosition.z, 1.0f);
//
//					if (light.Type == BlackJawz::Component::LightType::Directional ||
//						light.Type == BlackJawz::Component::LightType::Spot)
//					{
//						XMFLOAT3 lightRotation = lightTransform.GetRotation();
//						lcb.lights[lightIndex].LightDirection = lightRotation;
//					}
//				}
//
//				lcb.lights[lightIndex].LightType = static_cast<int>(light.Type);
//				lcb.lights[lightIndex].DiffuseLight = light.DiffuseLight;
//				lcb.lights[lightIndex].AmbientLight = light.AmbientLight;
//				lcb.lights[lightIndex].SpecularLight = light.SpecularLight;
//
//				lcb.lights[lightIndex].SpecularPower = light.SpecularPower;
//				lcb.lights[lightIndex].Range = light.Range;
//				lcb.lights[lightIndex].Intensity = light.Intensity;
//				lcb.lights[lightIndex].Attenuation = light.Attenuation;
//				lcb.lights[lightIndex].SpotInnerCone = light.SpotInnerCone;
//				lcb.lights[lightIndex].SpotOuterCone = light.SpotOuterCone;
//
//				lightIndex++;
//			}
//		}
//
//		pImmediateContext.Get()->UpdateSubresource(pLightsBuffer.Get(), 0, nullptr, &lcb, 0, 0);
//	}
//
//	// Iterate over entities in the Appearance System 
//	for (auto entity : appearanceSystem.GetEntities())
//	{
//		auto& appearance = appearanceSystem.GetAppearance(entity);
//		BlackJawz::Component::Geometry geo = appearance.GetGeometry();
//		ComPtr<ID3D11ShaderResourceView> entityTexture = appearance.GetTexture();
//
//		// --- Update Transform for this entity ---
//		if (transformSystem.HasComponent(entity))
//		{
//			auto& transform = transformSystem.GetTransform(entity);
//			tcb.World = XMMatrixTranspose(transform.GetWorldMatrix());
//		}
//
//		// Upload per-object constant buffer (transform, but not lighting)
//		pImmediateContext.Get()->UpdateSubresource(pTransformBuffer.Get(), 0, nullptr, &tcb, 0, 0);
//
//		// Bind Vertex and Index Buffers
//		pImmediateContext.Get()->IASetVertexBuffers(0, 1, geo.pVertexBuffer.GetAddressOf(), &geo.vertexBufferStride, &geo.vertexBufferOffset);
//		pImmediateContext.Get()->IASetIndexBuffer(geo.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
//
//		pImmediateContext.Get()->PSSetShaderResources(0, 1, entityTexture.GetAddressOf());
//
//		// Draw the entity
//		pImmediateContext.Get()->DrawIndexed(geo.IndicesCount, 0, 0);
//	}
//}

void BlackJawz::Rendering::Render::Draw(BlackJawz::System::TransformSystem& transformSystem,
	BlackJawz::System::AppearanceSystem& appearanceSystem, BlackJawz::System::LightSystem& lightSystem)
{
	// GBuffer Pass
	GBufferPass(transformSystem, appearanceSystem, lightSystem);

	// Lighting Pass
	LightingPass(lightSystem, transformSystem);

	// Quad Pass
	QuadPass();
}

void BlackJawz::Rendering::Render::GBufferPass(BlackJawz::System::TransformSystem& transformSystem,
	BlackJawz::System::AppearanceSystem& appearanceSystem, BlackJawz::System::LightSystem& lightSystem)
{
	// Bind G-Buffer Render Targets
	BeginGBufferPass();

	transformSystem.Update();

	pImmediateContext.Get()->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	ID3D11RenderTargetView* rtvs[4] =
	{
		gBufferRTVs[0].Get(),
		gBufferRTVs[1].Get(),
		gBufferRTVs[2].Get(),
		gBufferRTVs[3].Get()
	};

	pImmediateContext.Get()->OMSetRenderTargets(4, rtvs, pDepthStencilView.Get());

	TransformBuffer cb = {};

	// Load View and Projection Matrices
	XMMATRIX view = XMLoadFloat4x4(&viewMatrix);
	XMMATRIX projection = XMLoadFloat4x4(&projectionMatrix);
	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);

	// Bind Geometry Shaders (Deferred GBuffer Pass)
	pImmediateContext.Get()->IASetInputLayout(pGBufferInputLayout.Get());
	pImmediateContext.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pImmediateContext.Get()->VSSetShader(pGBufferVertexShader.Get(), nullptr, 0);
	pImmediateContext.Get()->VSSetConstantBuffers(0, 1, pTransformBuffer.GetAddressOf());

	pImmediateContext.Get()->PSSetShader(pGBufferPixelShader.Get(), nullptr, 0);
	pImmediateContext.Get()->PSSetSamplers(0, 1, pSamplerLinear.GetAddressOf());

	// Iterate over entities in the Appearance System (Geometry Pass)
	for (auto entity : appearanceSystem.GetEntities())
	{
		auto& appearance = appearanceSystem.GetAppearance(entity);
		BlackJawz::Component::Geometry geo = appearance.GetGeometry();
		ComPtr<ID3D11ShaderResourceView> entityTexture = appearance.GetTexture();

		// --- Update Transform for this entity ---
		if (transformSystem.HasComponent(entity))
		{
			auto& transform = transformSystem.GetTransform(entity);
			cb.World = XMMatrixTranspose(transform.GetWorldMatrix());
		}

		// Upload per-object constant buffer (transform)
		pImmediateContext.Get()->UpdateSubresource(pTransformBuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Bind Vertex and Index Buffers
		pImmediateContext.Get()->IASetVertexBuffers(0, 1, geo.pVertexBuffer.GetAddressOf(), &geo.vertexBufferStride, &geo.vertexBufferOffset);
		pImmediateContext.Get()->IASetIndexBuffer(geo.pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		pImmediateContext.Get()->PSSetShaderResources(0, 1, entityTexture.GetAddressOf());

		// Draw the entity (G-Buffer pass)
		pImmediateContext.Get()->DrawIndexed(geo.IndicesCount, 0, 0);
	}

	EndGBufferPass();
}

void BlackJawz::Rendering::Render::LightingPass(BlackJawz::System::LightSystem& lightSystem,
	BlackJawz::System::TransformSystem& transformSystem)
{
	lightSystem.Update();

	// Bind Lighting Render Target
	pImmediateContext.Get()->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	pImmediateContext.Get()->OMSetRenderTargets(1, g_pGbufferRenderLightingTargetView.GetAddressOf(), nullptr);

	LightsBuffer cb = {};

	cb.CameraPosition = cameraPosition;

	// Process All Lights
	if (!lightSystem.GetEntities().empty())
	{
		ZeroMemory(cb.lights, sizeof(cb.lights));
		cb.numLights = static_cast<int>(lightSystem.GetEntities().size());

		int lightIndex = 0;
		for (auto lightEntity : lightSystem.GetEntities())
		{
			if (lightIndex >= MAX_LIGHTS) break;

			auto& light = lightSystem.GetLight(lightEntity);

			if (transformSystem.HasComponent(lightEntity))
			{
				auto& lightTransform = transformSystem.GetTransform(lightEntity);
				XMFLOAT3 lightPosition = lightTransform.GetPosition();
				cb.lights[lightIndex].LightPosition = XMFLOAT4(lightPosition.x, lightPosition.y, lightPosition.z, 1.0f);

				if (light.Type == BlackJawz::Component::LightType::Directional ||
					light.Type == BlackJawz::Component::LightType::Spot)
				{
					cb.lights[lightIndex].LightDirection = lightTransform.GetRotation();
				}
			}

			cb.lights[lightIndex].LightType = static_cast<int>(light.Type);
			cb.lights[lightIndex].DiffuseLight = light.DiffuseLight;
			cb.lights[lightIndex].AmbientLight = light.AmbientLight;
			cb.lights[lightIndex].SpecularLight = light.SpecularLight;

			cb.lights[lightIndex].SpecularPower = light.SpecularPower;
			cb.lights[lightIndex].Range = light.Range;
			cb.lights[lightIndex].Intensity = light.Intensity;
			cb.lights[lightIndex].Attenuation = light.Attenuation;
			cb.lights[lightIndex].SpotInnerCone = light.SpotInnerCone;
			cb.lights[lightIndex].SpotOuterCone = light.SpotOuterCone;

			lightIndex++;
		}

		pImmediateContext.Get()->UpdateSubresource(pLightsBuffer.Get(), 0, nullptr, &cb, 0, 0);
	}

	UINT stride = sizeof(VertexQuad);
	UINT offset = 0;
	pImmediateContext.Get()->IASetVertexBuffers(0, 1, pDeferredQuadVB.GetAddressOf(), &stride, &offset);
	pImmediateContext.Get()->IASetIndexBuffer(pDeferredQuadIB.Get(), DXGI_FORMAT_R16_UINT, 0);
	pImmediateContext.Get()->IASetInputLayout(pDeferredLightingInputLayout.Get());

	// Bind Deferred Lighting Shaders
	pImmediateContext.Get()->VSSetShader(pDeferredLightingVertexShader.Get(), nullptr, 0);

	pImmediateContext.Get()->PSSetShader(pDeferredLightingPixelShader.Get(), nullptr, 0);
	pImmediateContext.Get()->PSSetConstantBuffers(0, 1, pLightsBuffer.GetAddressOf());
	pImmediateContext.Get()->PSSetSamplers(0, 1, pSamplerLinear.GetAddressOf());

	ID3D11ShaderResourceView* srvs[4] =
	{
		gBufferSRVs[0].Get(),
		gBufferSRVs[1].Get(),
		gBufferSRVs[2].Get(),
		gBufferSRVs[3].Get()
	};

	pImmediateContext->PSSetShaderResources(0, 4, srvs);

	// Draw Fullscreen Quad
	pImmediateContext.Get()->DrawIndexed(6, 0, 0);
}

void BlackJawz::Rendering::Render::QuadPass()
{
	// Bind Backbuffer
	pImmediateContext.Get()->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	pImmediateContext.Get()->OMSetRenderTargets(1, pQuadRenderTargetView.GetAddressOf(), nullptr);

	UINT stride = sizeof(VertexQuad);
	UINT offset = 0;
	// Set Quad Buffers
	pImmediateContext.Get()->IASetVertexBuffers(0, 1, pDeferredQuadVB.GetAddressOf(), &stride, &offset);
	pImmediateContext.Get()->IASetIndexBuffer(pDeferredQuadIB.Get(), DXGI_FORMAT_R16_UINT, 0);
	pImmediateContext.Get()->IASetInputLayout(pPostProcessingInputLayout.Get());

	// Set Quad Shaders
	pImmediateContext.Get()->VSSetShader(pPostProcessingVertexShader.Get(), nullptr, 0);
	pImmediateContext.Get()->PSSetShader(pPostProcessingPixelShader.Get(), nullptr, 0);

	pImmediateContext.Get()->PSSetSamplers(0, 1, pSamplerLinear.GetAddressOf());

	// Bind Lighting Pass Result
	ID3D11ShaderResourceView* finalSRV[] = { g_pGbufferShaderResourceLightingView.Get() };
	pImmediateContext->PSSetShaderResources(0, 1, finalSRV);

	// Render Fullscreen Quad
	pImmediateContext.Get()->DrawIndexed(6, 0, 0);

}

void BlackJawz::Rendering::Render::EndFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	pSwapChain.Get()->Present(1, 0);

	// Set Shader Resource to Null / Clear
	ID3D11ShaderResourceView* const shaderClear[1] = { nullptr };
	for (int i = 0; i < 6; i++)
	{
		pImmediateContext.Get()->PSSetShaderResources(i, 1, shaderClear);
	}
}

void BlackJawz::Rendering::Render::CleanUp()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}