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
	sd.BufferDesc.Width = BlackJawz::Application::Application::GetWindowWidth();
	sd.BufferDesc.Height = BlackJawz::Application::Application::GetWindowHeight();
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

HRESULT BlackJawz::Rendering::Render::InitRenderTargetView()
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
	textureDesc.Width = BlackJawz::Application::Application::GetWindowWidth();
	textureDesc.Height = BlackJawz::Application::Application::GetWindowHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

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

void BlackJawz::Rendering::Render::ResizeRenderTarget(int width, int height)
{
	if (width == renderWidth && height == renderHeight)
	{
		return;
	}

	pRenderTargetTextureView.Reset();
	pRenderTexture.Reset();
	pShaderResourceView.Reset();

	renderWidth = width;
	renderHeight = height;

	// Recreate the render target texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = renderWidth;
	textureDesc.Height = renderHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = pID3D11Device.Get()->CreateTexture2D(&textureDesc, nullptr, pRenderTexture.GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to resize render target texture.");
	}

	// Recreate the render target view
	hr = pID3D11Device.Get()->CreateRenderTargetView(pRenderTexture.Get(), nullptr, pRenderTargetTextureView.GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create render target view.");
	}

	// Recreate the shader resource view
	hr = pID3D11Device.Get()->CreateShaderResourceView(pRenderTexture.Get(), nullptr, pShaderResourceView.GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create shader resource view.");
	}

}

HRESULT BlackJawz::Rendering::Render::InitViewPort()
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)BlackJawz::Application::Application::GetWindowWidth();
	vp.Height = (FLOAT)BlackJawz::Application::Application::GetWindowHeight();
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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

	depthStencilDesc.Width = BlackJawz::Application::Application::GetWindowWidth();
	depthStencilDesc.Height = BlackJawz::Application::Application::GetWindowHeight();
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

	D3D11_DEPTH_STENCIL_DESC dssDesc = {};
	dssDesc.DepthEnable = true; // Disable depth testing for a simple triangle
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS;

	ComPtr<ID3D11DepthStencilState> depthStencilState;
	hr = pID3D11Device->CreateDepthStencilState(&dssDesc, depthStencilState.GetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth-stencil state.\n");
		return hr;
	}

	pImmediateContext.Get()->OMSetDepthStencilState(depthStencilState.Get(), 1);

	return hr;
}

HRESULT BlackJawz::Rendering::Render::InitRasterizer()
{
	// Rasterizer
	HRESULT hr = S_OK;
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;
	hr = pID3D11Device.Get()->CreateRasterizerState(&cmdesc, RSCullNone.GetAddressOf());

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	pID3D11Device.Get()->CreateDepthStencilState(&dssDesc, DSLessEqual.GetAddressOf());

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	hr = pID3D11Device.Get()->CreateRasterizerState(&cmdesc, CCWcullMode.GetAddressOf());

	cmdesc.FrontCounterClockwise = false;
	cmdesc.DepthClipEnable = true;
	cmdesc.ScissorEnable = false;
	cmdesc.MultisampleEnable = false;
	cmdesc.AntialiasedLineEnable = false;
	hr = pID3D11Device.Get()->CreateRasterizerState(&cmdesc, CWcullMode.GetAddressOf());

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

	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(ConstantBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	hr = pID3D11Device.Get()->CreateBuffer(&bufferDesc, nullptr, pConstantBuffer.GetAddressOf());
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
		// Top face (Gradient from Red -> Green -> Blue -> Red)
	{ {-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }, // Red
	{ { 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }, // Green
	{ { 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }, // Blue
	{ {-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }, // Red

	// Bottom face (Gradient from Red -> Blue -> Green -> Red)
	{ {-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }, // Red
	{ { 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }, // Blue
	{ { 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }, // Green
	{ {-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }, // Red

	// Left face (Gradient from Green -> Blue -> Red -> Green)
	{ {-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }, // Green
	{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }, // Blue
	{ {-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }, // Red
	{ {-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }, // Green

	// Right face (Gradient from Blue -> Red -> Green -> Blue)
	{ { 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }, // Blue
	{ { 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }, // Red
	{ { 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }, // Green
	{ { 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }, // Blue

	// Front face (Gradient from Green -> Blue -> Red -> Green)
	{ {-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }, // Green
	{ { 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }, // Blue
	{ { 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }, // Red
	{ {-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }, // Green

	// Back face (Gradient from Blue -> Green -> Red -> Blue)
	{ {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }, // Blue
	{ { 1.0f, -1.0f,  1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }, // Green
	{ { 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} }, // Red
	{ {-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }, // Blue
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
	WORD indices[] =
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
	bd.ByteWidth = sizeof(WORD) * 36;
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

			vertex.position.x = radius * sin(phi) * cos(theta);
			vertex.position.y = radius * cos(phi);
			vertex.position.z = radius * sin(phi) * sin(theta);

			vertex.color.x = vertex.position.x / radius; // Red based on x
			vertex.color.y = vertex.position.y / radius; // Green based on y
			vertex.color.z = vertex.position.z / radius; // Blue based on z

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
			sphereIndices.push_back(v2);
			sphereIndices.push_back(v1);

			sphereIndices.push_back(v1);
			sphereIndices.push_back(v2);
			sphereIndices.push_back(v3);
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
	ibDesc.ByteWidth = sizeof(WORD) * sphereIndices.size();
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

	// Create vertex buffer
	Vertex vertices[] =
	{
	{ {-1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	{ {-1.0f, 0.0f, -1.0f  }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	{ {1.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f, 1.0f } },
	{ {1.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = pID3D11Device.Get()->CreateBuffer(&bd, &InitData, pPlaneVertexBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	// Create index buffer
	WORD indices[] =
	{
		0, 2, 1,
		2, 3, 1,
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pID3D11Device.Get()->CreateBuffer(&bd, &InitData, pPlaneIndexBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;

	return hr; // Return S_OK if everything succeeded
}

HRESULT BlackJawz::Rendering::Render::Initialise()
{
	if (FAILED(InitDeviceAndSwapChain()))
	{
		return E_FAIL;
	}

	if (FAILED(InitRenderTargetView()))
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

	InitPlane();


	return S_OK;
}

void BlackJawz::Rendering::Render::RenderCube(UINT newCount)
{
	if (newCount == cubeCount)
		return; // No change needed

	if (newCount > cubeCount)
	{
		// Add new cubes
		for (int i = cubeCount; i < newCount; i++)
		{
			BlackJawz::GameObject::Appearance::Geometry cubeGeometry;
			cubeGeometry.pIndexBuffer = pCubeIndexBuffer;
			cubeGeometry.pVertexBuffer = pCubeVertexBuffer;
			cubeGeometry.IndicesCount = 36;
			cubeGeometry.vertexBufferOffset = 0;
			cubeGeometry.vertexBufferStride = sizeof(Vertex);

			BlackJawz::GameObject::GameObject* gameObjectCube = new BlackJawz::GameObject::GameObject("Cube " + i, cubeGeometry);
			gameObjectCube->GetTransform()->SetPosition(-2.0f + (i * 2.0f), 0.0f, 5.0f);
			gameObjectCube->GetTransform()->SetScale(0.5f, 0.5f, 0.5f);
			gameObjectCube->GetTransform()->SetRotation(15.0f, 15.0f, 0.0f);

			pGameObjectListCube.push_back(gameObjectCube);
		}
	}
	else if (newCount < cubeCount)
	{
		// Remove extra cubes
		for (int i = cubeCount; i > newCount; i--)
		{
			delete pGameObjectListCube.back(); // Clean up memory for removed cubes
			pGameObjectListCube.pop_back();
		}
	}

	cubeCount = newCount; // Update the count
}

void BlackJawz::Rendering::Render::RenderSphere(UINT newCount)
{
	if (newCount == sphereCount)
		return; // No change needed

	if (newCount > sphereCount)
	{
		// Add new cubes
		for (int i = sphereCount; i < newCount; i++)
		{
			BlackJawz::GameObject::Appearance::Geometry sphereGeometry;
			sphereGeometry.pIndexBuffer = pSphereIndexBuffer;
			sphereGeometry.pVertexBuffer = pSphereVertexBuffer;
			sphereGeometry.IndicesCount = sphereIndices.size();
			sphereGeometry.vertexBufferOffset = 0;
			sphereGeometry.vertexBufferStride = sizeof(Vertex);

			BlackJawz::GameObject::GameObject* gameObjectSphere = new BlackJawz::GameObject::GameObject("Sphere " + i, sphereGeometry);
			gameObjectSphere->GetTransform()->SetPosition(-2.0f + (i * 2.0f), 1.5f, 5.0f);
			gameObjectSphere->GetTransform()->SetScale(0.5f, 0.5f, 0.5f);
			gameObjectSphere->GetTransform()->SetRotation(0.0f, 0.0f, 0.0f);

			pGameObjectListSphere.push_back(gameObjectSphere);
		}
	}
	else if (newCount < sphereCount)
	{
		// Remove extra cubes
		for (int i = sphereCount; i > newCount; i--)
		{
			delete pGameObjectListSphere.back(); // Clean up memory for removed cubes
			pGameObjectListSphere.pop_back();
		}
	}

	sphereCount = newCount; // Update the count
}

void BlackJawz::Rendering::Render::RenderPlane(UINT newCount)
{
	if (newCount == planeCount)
		return; // No change needed

	if (newCount > planeCount)
	{
		// Add new cubes
		for (int i = planeCount; i < newCount; i++)
		{
			BlackJawz::GameObject::Appearance::Geometry planeGeometry;
			planeGeometry.pIndexBuffer = pPlaneIndexBuffer;
			planeGeometry.pVertexBuffer = pPlaneVertexBuffer;
			planeGeometry.IndicesCount = 6;
			planeGeometry.vertexBufferOffset = 0;
			planeGeometry.vertexBufferStride = sizeof(Vertex);

			BlackJawz::GameObject::GameObject* gameObjectPlane = new BlackJawz::GameObject::GameObject("Plane " + i, planeGeometry);
			gameObjectPlane->GetTransform()->SetPosition(0.0f, -3.0f, 10.0f + (i * 20.0f));
			gameObjectPlane->GetTransform()->SetScale(10.0f, 10.0f, 10.0f);
			gameObjectPlane->GetTransform()->SetRotation(0.0f, 0.0f, 0.0f);

			pGameObjectListPlane.push_back(gameObjectPlane);
		}
	}
	else if (newCount < planeCount)
	{
		// Remove extra cubes
		for (int i = planeCount; i > newCount; i--)
		{
			delete pGameObjectListPlane.back(); // Clean up memory for removed cubes
			pGameObjectListPlane.pop_back();
		}
	}

	planeCount = newCount; // Update the count
}

void BlackJawz::Rendering::Render::RenderToTexture()
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

	// Render 3D scene
	Draw();

	// Restore the default render target after rendering to texture
	pImmediateContext.Get()->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);
}

void BlackJawz::Rendering::Render::Update()
{
	for (auto gameObjects : pGameObjectListCube)
	{
		gameObjects->Update();
	}

	for (auto gameObjects : pGameObjectListSphere)
	{
		gameObjects->Update();
	}

	for (auto gameObjects : pGameObjectListPlane)
	{
		gameObjects->Update();
	}
}

void BlackJawz::Rendering::Render::BeginFrame()
{
	// Clear the back buffer
	SetBackGroundColour(0.0f, 0.0f, 0.0f, 1.0f);
	pImmediateContext.Get()->ClearRenderTargetView(pRenderTargetView.Get(), ClearColor);
	pImmediateContext.Get()->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	pImmediateContext.Get()->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);
}

void BlackJawz::Rendering::Render::Draw()
{
	// Scene Render
	ConstantBuffer cb = {};

	XMMATRIX view = XMLoadFloat4x4(&viewMatrix);
	XMMATRIX projection = XMLoadFloat4x4(&projectionMatrix);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);

	pImmediateContext.Get()->IASetInputLayout(pInputLayout.Get());
	pImmediateContext.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pImmediateContext.Get()->VSSetShader(pVertexShader.Get(), nullptr, 0);
	pImmediateContext.Get()->VSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());

	pImmediateContext.Get()->PSSetShader(pPixelShader.Get(), nullptr, 0);
	pImmediateContext.Get()->PSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());

	pImmediateContext.Get()->PSSetSamplers(0, 1, pSamplerLinear.GetAddressOf());

	for (auto gameObjectsCube : pGameObjectListCube)
	{
		cb.World = XMMatrixTranspose(gameObjectsCube->GetTransform()->GetWorldMatrix());
		pImmediateContext.Get()->UpdateSubresource(pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

		gameObjectsCube->Draw(pImmediateContext.Get());
	}

	for (auto gameObjectsSphere : pGameObjectListSphere)
	{
		cb.World = XMMatrixTranspose(gameObjectsSphere->GetTransform()->GetWorldMatrix());
		pImmediateContext.Get()->UpdateSubresource(pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

		gameObjectsSphere->Draw(pImmediateContext.Get());
	}

	for (auto gameObjectsPlane : pGameObjectListPlane)
	{
		cb.World = XMMatrixTranspose(gameObjectsPlane->GetTransform()->GetWorldMatrix());
		pImmediateContext.Get()->UpdateSubresource(pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

		gameObjectsPlane->Draw(pImmediateContext.Get());
	}
}

void BlackJawz::Rendering::Render::EndFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	pSwapChain.Get()->Present(1, 0);
}

void BlackJawz::Rendering::Render::CleanUp()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}