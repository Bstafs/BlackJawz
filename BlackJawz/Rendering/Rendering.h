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
};

struct VertexQuad
{
	XMFLOAT3 Position;
	XMFLOAT2 TexC;
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

struct ConstantBuffer
{
	XMMATRIX World; 
	XMMATRIX View;
	XMMATRIX Projection;	

	LightProperties lights[MAX_LIGHTS];

	int numLights;
	XMFLOAT3 CameraPosition;
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
		ID3D11ShaderResourceView* GetShaderResourceView() const { return  pShaderResourceView.Get(); } // For ImGui::Image
		void ResizeRenderTarget(int width, int height);

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
		HRESULT InitRenderTargetView();
		HRESULT InitViewPort();
		HRESULT InitShadersAndInputLayout();
		HRESULT InitGBufferShadersAndInputLayout();
		HRESULT InitDeferredLightingShaders();
		HRESULT InitPostProcessingShaders();
		HRESULT InitSamplerState();
		HRESULT InitDepthStencil();
		HRESULT InitRasterizer();
		HRESULT InitImGui();
		HRESULT InitConstantBuffer();
		HRESULT InitCube();
		HRESULT InitSphere();
		HRESULT InitPlane();

		// Deferred Shading
		HRESULT InitGBuffer();
		HRESULT InitDeferredQuad();
		void BeginGBufferPass();
		void EndGBufferPass();
		void LightingPass(BlackJawz::System::LightSystem& lightSystem, BlackJawz::System::TransformSystem& transformSystem);
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

		// Render Target
		ComPtr<ID3D11RenderTargetView> pRenderTargetView;

		// Render To Texture
		ComPtr<ID3D11Texture2D> pRenderTexture;
		ComPtr<ID3D11RenderTargetView> pRenderTargetTextureView;
		ComPtr<ID3D11ShaderResourceView> pShaderResourceView;

		// Depth Buffer
		ComPtr<ID3D11DepthStencilView> pDepthStencilView;
		ComPtr<ID3D11Texture2D> pDepthStencilBuffer;

		ComPtr<ID3D11DepthStencilState> DSLessEqual;
		ComPtr<ID3D11RasterizerState> RSCullNone;
		ComPtr<ID3D11RasterizerState> CCWcullMode;
		ComPtr<ID3D11RasterizerState> CWcullMode;

		int renderWidth = BlackJawz::Application::Application::GetWindowWidth();
		int renderHeight = BlackJawz::Application::Application::GetWindowHeight();

		// Cube
		ComPtr<ID3D11Buffer> pCubeVertexBuffer;
		ComPtr<ID3D11Buffer> pCubeIndexBuffer;

		// Sphere
		ComPtr<ID3D11Buffer> pSphereVertexBuffer;
		ComPtr<ID3D11Buffer> pSphereIndexBuffer;
		std::vector<Vertex> sphereVertices;
		std::vector<WORD> sphereIndices;

		// Plane
		ComPtr<ID3D11Buffer> pPlaneVertexBuffer;
		ComPtr<ID3D11Buffer> pPlaneIndexBuffer;

		// Constant Buffer
		ComPtr<ID3D11Buffer> pConstantBuffer;

		// Camera
		XMFLOAT4X4 viewMatrix = XMFLOAT4X4();
		XMFLOAT4X4 projectionMatrix = XMFLOAT4X4();
		XMFLOAT3 cameraPosition = XMFLOAT3();

		ID3D11ShaderResourceView* textureRV;

		// Deferred Rendering
		ComPtr<ID3D11Texture2D> gBufferTextures[4];      // Albedo, Normal, Position, Specular
		ComPtr<ID3D11RenderTargetView> gBufferRTVs[4];   // Render Target Views
		ComPtr<ID3D11ShaderResourceView> gBufferSRVs[4]; // Shader Resource Views

		ComPtr<ID3D11Texture2D> gBufferDepth;           // Depth Texture
		ComPtr<ID3D11DepthStencilView> gBufferDepthDSV; // Depth Stencil View
		ComPtr<ID3D11ShaderResourceView> gBufferDepthSRV; // Depth SRV

		// GBuffer 
		ComPtr<ID3D11VertexShader> pGBufferVertexShader;
		ComPtr<ID3D11PixelShader> pGBufferPixelShader;
		ComPtr<ID3D11InputLayout> pGBufferInputLayout;

		// Lighting
		ComPtr<ID3D11VertexShader> pDeferredLightingVertexShader;
		ComPtr<ID3D11PixelShader> pDeferredLightingPixelShader;
		ComPtr<ID3D11InputLayout> pDeferredLightingInputLayout;

		// Quad
		ComPtr<ID3D11Buffer> pDeferredQuadVB;
		ComPtr<ID3D11Buffer> pDeferredQuadIB;
		ComPtr<ID3D11VertexShader> pPostProcessingVertexShader;
		ComPtr<ID3D11PixelShader> pPostProcessingPixelShader;
		ComPtr<ID3D11InputLayout> pPostProcessingInputLayout;


		ID3D11Texture2D* g_pGbufferTargetLightingTextures = nullptr;
		ID3D11RenderTargetView* g_pGbufferRenderLightingTargetView = nullptr;
		ID3D11ShaderResourceView* g_pGbufferShaderResourceLightingView = nullptr;

		ID3D11BlendState* blendState = nullptr;
	};
}