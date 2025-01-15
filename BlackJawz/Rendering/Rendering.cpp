#include "Rendering.h"

BlackJawz::Rendering::Render::Render()
{
    _driverType = D3D_DRIVER_TYPE_NULL;
    _featureLevel = D3D_FEATURE_LEVEL_11_0;
}

BlackJawz::Rendering::Render::~Render()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
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
    sd.BufferDesc.RefreshRate.Numerator = 165;
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

    return hr;
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

    return S_OK;
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
    hr = pID3D11Device.Get()->CreateRasterizerState(&cmdesc, CWcullMode.GetAddressOf());

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

	return S_OK;
}

void BlackJawz::Rendering::Render::Update()
{

}

void BlackJawz::Rendering::Render::Draw()
{
    pImmediateContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());

    SetBackGroundColour(0.1f, 0.5f, 1.0f, 1.0f);
    pImmediateContext->ClearRenderTargetView(pRenderTargetView.Get(), ClearColor);
    pImmediateContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    pSwapChain.Get()->Present(1, 0);
}
