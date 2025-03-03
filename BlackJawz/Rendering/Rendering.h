#pragma once
#include "../pch.h"
#include "../Windows/Application.h"
#include "../ECS/Components.h"
#include "../ECS/Systems.h"

#define MAX_LIGHTS 10

namespace BlackJawz::Application
{
	class Application;
}

struct Vertex
{
	DirectX::XMFLOAT3 Position; // Vertex Position
	DirectX::XMFLOAT3 Normal;   // Vertex Normals
	DirectX::XMFLOAT2 TexC;     // Vertex Texture Coordinates
	DirectX::XMFLOAT3 Tangent;     // Vertex Tangents
};

struct VertexQuad
{
	XMFLOAT3 Position;
	XMFLOAT2 TexC;
};

struct SkyboxVertex
{
	XMFLOAT3 Position;
};

struct LightProperties
{
	XMFLOAT4 LightPosition;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 SpecularLight;

	float SpecularPower;
	float Range;
	XMFLOAT2 Padding01;

	XMFLOAT3 LightDirection;
	float Intensity;

	XMFLOAT3 Attenuation;
	float Padding02;

	float SpotInnerCone;
	float SpotOuterCone;
	int LightType;
	float Padding03;
};

struct TransformBuffer
{
	XMMATRIX World; 
	XMMATRIX View;
	XMMATRIX Projection;	
};

struct LightsBuffer
{
	LightProperties lights[MAX_LIGHTS];

	int numLights;
	XMFLOAT3 CameraPosition;
};

struct PostProcessingBuffer
{
	XMFLOAT2 ScreenSize;   // Screen resolution (e.g., 1920x1080)
	XMFLOAT2 RcpScreenSize; // 1.0 / ScreenSize (precomputed)
	float ContrastThreshold; // Edge detection threshold (e.g., 0.0312)
	float RelativeThreshold; // Relative threshold (e.g., 0.063)
	float SubpixelBlending; // Subpixel blending strength (e.g., 0.75)
	float Padding01;
};

namespace BlackJawz::Rendering
{
	class Render
	{
	public:
		Render();
		~Render();

		HRESULT Initialise();
		void Update();
		void Draw(BlackJawz::System::TransformSystem& transformSystem,
			BlackJawz::System::AppearanceSystem& appearanceSystem, BlackJawz::System::LightSystem& lightSystem);

		void SetBackGroundColour(float r, float g, float b, float a) { ClearColor[0] = r, ClearColor[1] = g, ClearColor[2] = b, ClearColor[3] = a; };
		void BeginFrame();
		void EndFrame();

		void RenderToTexture(BlackJawz::System::TransformSystem& transformSystem,
			BlackJawz::System::AppearanceSystem& appearanceSystem, BlackJawz::System::LightSystem& lightSystem); // Render scene to the texture
		ID3D11ShaderResourceView* GetShaderResourceView() const { return  pQuadRenderShaderResourceView.Get(); } // For ImGui::Image
		void ResizeRenderTarget(int width, int height);
		void ResizeBackBuffer();
		void ResizeGBuffer();
		void ResizeDepthStencilBuffer();
		void ResizeSkyBox();

		void SetViewMatrix(XMFLOAT4X4 viewmatrix) { viewMatrix = viewmatrix; }
		void SetProjectionMatrix(XMFLOAT4X4 projMatrix) { projectionMatrix = projMatrix; }
		void SetCameraPosition(XMFLOAT3 cameraPos) { cameraPosition = cameraPos; }

		ID3D11DeviceContext* GetDeviceContext() { return pImmediateContext.Get(); }
		ID3D11Device* GetDevice() { return pID3D11Device.Get(); }

		BlackJawz::Component::Geometry CreateCubeGeometry();
		BlackJawz::Component::Geometry CreateSphereGeometry();
		BlackJawz::Component::Geometry CreatePlaneGeometry();

		void CleanUp();

	private:
		HRESULT InitDeviceAndSwapChain();

		HRESULT InitRenderTargetViews();
		HRESULT InitBackBuffer();
		HRESULT InitBRDFLUTView();
		HRESULT InitIrradianceView();
		HRESULT InitRadianceView();
		HRESULT InitLightingView();
		HRESULT InitSkyboxView();
		HRESULT InitQuadView();

		HRESULT InitViewPort();
		HRESULT InitShadersAndInputLayout();
		HRESULT InitGBufferShadersAndInputLayout();
		HRESULT InitBRDFLUTShadersAndInputLayout();
		HRESULT InitIrradianceShadersAndInputLayout();
		HRESULT InitRadianceShadersAndInputLayout();

		HRESULT InitDeferredLightingShaders();
		HRESULT InitSkyBoxShadersAndInputLayout();
		HRESULT InitPostProcessingShaders();
		HRESULT InitSamplerState();
		HRESULT InitShaderMapping();
		HRESULT InitDepthStencil();
		HRESULT InitRasterizer();
		HRESULT InitImGui();
		HRESULT InitConstantBuffer();
		HRESULT InitCube();
		HRESULT InitSphere();
		HRESULT InitPlane();
		HRESULT InitSkyBox();

		// Deferred Shading PBR with Radiance, Irradiance and BRDFLUT
		HRESULT InitGBuffer();
		HRESULT InitDeferredQuad();
		void BeginGBufferPass();
		void GBufferPass(BlackJawz::System::TransformSystem& transformSystem,
			BlackJawz::System::AppearanceSystem& appearanceSystem, BlackJawz::System::LightSystem& lightSystem);
		void EndGBufferPass();
		void PreComputeBRDFLUT();
		void PreComputeRadiance();
		void PreComputeIrradiance();
		void LightingPass(BlackJawz::System::LightSystem& lightSystem, BlackJawz::System::TransformSystem& transformSystem);
		void SkyBox();
		void QuadPass();

	private:
		ComPtr<ID3D11Device> pID3D11Device;
		ComPtr<ID3D11DeviceContext> pImmediateContext;
		ComPtr<IDXGISwapChain> pSwapChain;

		D3D_DRIVER_TYPE         _driverType;
		D3D_FEATURE_LEVEL       _featureLevel;

		// Shaders
		ComPtr<ID3D11VertexShader> pVertexShader;
		ComPtr<ID3D11PixelShader> pPixelShader;
		ComPtr<ID3D11InputLayout> pInputLayout;

		UINT mSampleCount = 1;
		float ClearColor[4] = {};

		ComPtr<ID3D11SamplerState> pSamplerLinear;
		ComPtr<ID3D11SamplerState> pSamplerCube;

		// Render Target
		ComPtr<ID3D11RenderTargetView> pRenderTargetView;

		// Render To Texture
		ComPtr<ID3D11Texture2D> pRenderTexture;
		ComPtr<ID3D11RenderTargetView> pRenderTargetTextureView;
		ComPtr<ID3D11ShaderResourceView> pShaderResourceView;

		// Depth Buffer
		ComPtr<ID3D11DepthStencilView> pDepthStencilView;
		ComPtr<ID3D11Texture2D> pDepthStencilBuffer;
		ComPtr<ID3D11ShaderResourceView> pDepthStencilShaderResourceView;

		ComPtr<ID3D11DepthStencilState> DSLess;
		ComPtr<ID3D11DepthStencilState> DSLessEqual;
		ComPtr<ID3D11RasterizerState> RSCullNone;
		ComPtr<ID3D11RasterizerState> CCWcullMode;
		ComPtr<ID3D11RasterizerState> CWcullMode;
		ComPtr<ID3D11RasterizerState> CWcullModeFront;

		int renderWidth = BlackJawz::Application::Application::GetWindowWidth();
		int renderHeight = BlackJawz::Application::Application::GetWindowHeight();

		// Cube
		ComPtr<ID3D11Buffer> pCubeVertexBuffer;
		ComPtr<ID3D11Buffer> pCubeIndexBuffer;

		// Sphere
		ComPtr<ID3D11Buffer> pSphereVertexBuffer;
		ComPtr<ID3D11Buffer> pSphereIndexBuffer;
		std::vector<Vertex> sphereVertices;
		std::vector<uint32_t> sphereIndices;

		// Plane
		ComPtr<ID3D11Buffer> pPlaneVertexBuffer;
		ComPtr<ID3D11Buffer> pPlaneIndexBuffer;
		std::vector<Vertex> gridVertices;
		std::vector<uint32_t> gridIndices;

		// Constant Buffer
		ComPtr<ID3D11Buffer> pTransformBuffer;
		ComPtr<ID3D11Buffer> pLightsBuffer;
		ComPtr<ID3D11Buffer> pPostProcessingBuffer;

		// Camera
		XMFLOAT4X4 viewMatrix = XMFLOAT4X4();
		XMFLOAT4X4 projectionMatrix = XMFLOAT4X4();
		XMFLOAT3 cameraPosition = XMFLOAT3();

		ID3D11ShaderResourceView* textureRV;

		// Deferred Rendering
		ComPtr<ID3D11Texture2D> gBufferTextures[4];      // Albedo, Normal, Position, Specular
		ComPtr<ID3D11RenderTargetView> gBufferRTVs[4];   // Render Target Views
		ComPtr<ID3D11ShaderResourceView> gBufferSRVs[4]; // Shader Resource Views

		// GBuffer 
		ComPtr<ID3D11VertexShader> pGBufferVertexShader;
		ComPtr<ID3D11PixelShader> pGBufferPixelShader;
		ComPtr<ID3D11InputLayout> pGBufferInputLayout;

		// Lighting
		ComPtr<ID3D11VertexShader> pDeferredLightingVertexShader;
		ComPtr<ID3D11PixelShader> pDeferredLightingPixelShader;
		ComPtr<ID3D11InputLayout> pDeferredLightingInputLayout;

		ComPtr<ID3D11Texture2D> g_pGbufferTargetLightingTextures;
		ComPtr<ID3D11RenderTargetView> g_pGbufferRenderLightingTargetView;
		ComPtr<ID3D11ShaderResourceView> g_pGbufferShaderResourceLightingView;

		// Quad
		ComPtr<ID3D11Buffer> pDeferredQuadVB;
		ComPtr<ID3D11Buffer> pDeferredQuadIB;
		ComPtr<ID3D11VertexShader> pPostProcessingVertexShader;
		ComPtr<ID3D11PixelShader> pPostProcessingPixelShader;
		ComPtr<ID3D11InputLayout> pPostProcessingInputLayout;

		ComPtr<ID3D11Texture2D> pQuadRenderTargetTexture;
		ComPtr<ID3D11RenderTargetView> pQuadRenderTargetView;
		ComPtr<ID3D11ShaderResourceView> pQuadRenderShaderResourceView;

		// SkyBox, Radiance Map, Irradiance Map
		ComPtr<ID3D11ShaderResourceView> texSkyBox;
		ComPtr<ID3D11ShaderResourceView> texRadianceMap;
		ComPtr<ID3D11ShaderResourceView> texIrradianceMap;
		ComPtr<ID3D11ShaderResourceView> texBRDFLUTMap;

		// BRDFLUT
		ComPtr<ID3D11VertexShader> pBRDFLUTVertexShader;
		ComPtr<ID3D11PixelShader> pBRDFLUTPixelShader;
		ComPtr<ID3D11InputLayout> pBRDFLUTInputLayout;

		ComPtr<ID3D11Texture2D> pBRDFLUTTexture;
		ComPtr<ID3D11RenderTargetView> pBRDFLUTRenderTargetView;
		ComPtr<ID3D11ShaderResourceView> pBRDFLUTShaderResourceView;

		// Irradiance
		ComPtr<ID3D11VertexShader> pIrradianceVertexShader;
		ComPtr<ID3D11PixelShader> pIrradiancePixelShader;
		ComPtr<ID3D11InputLayout> pIrradianceInputLayout;

		ComPtr<ID3D11Texture2D> pIrradianceTexture;
		ComPtr<ID3D11RenderTargetView> pIrradianceRenderTargetView;
		ComPtr<ID3D11ShaderResourceView> pIrradianceShaderResourceView;

		// Radiance
		ComPtr<ID3D11VertexShader> pRadianceVertexShader;
		ComPtr<ID3D11PixelShader> pRadiancePixelShader;
		ComPtr<ID3D11InputLayout> pRadianceInputLayout;

		ComPtr<ID3D11Texture2D> pRadianceTexture;
		ComPtr<ID3D11RenderTargetView> pRadianceRenderTargetView;
		ComPtr<ID3D11ShaderResourceView> pRadianceShaderResourceView;

		// Skybox
		ComPtr<ID3D11VertexShader> pSkyboxVertexShader;
		ComPtr<ID3D11PixelShader> pSkyboxPixelShader;
		ComPtr<ID3D11InputLayout> pSkyboxInputLayout;

		ComPtr<ID3D11Texture2D> pSkyboxTexture;
		ComPtr<ID3D11RenderTargetView> pSkyboxRenderTargetView;
		ComPtr<ID3D11ShaderResourceView> pSkyboxShaderResourceView;

		ComPtr<ID3D11Buffer> pSkyboxVertexBuffer;
		ComPtr<ID3D11Buffer> pSkyboxIndexBuffer;
		std::vector<Vertex> skyboxVertices;
		std::vector<uint32_t> skyboxIndices;
	};
}