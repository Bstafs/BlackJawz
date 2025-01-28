#pragma once
#include "../pch.h"
#include "../Windows/Application.h"
#include "GameObjects/GameObject.h"

namespace BlackJawz::Application
{
	class Application;
}

struct Vertex
{
	DirectX::XMFLOAT3 position; // Position of the vertex
	DirectX::XMFLOAT4 color;    // Color of the vertex
};

struct ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
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
		void Draw();

		void SetBackGroundColour(float r, float g, float b, float a) { ClearColor[0] = r, ClearColor[1] = g, ClearColor[2] = b, ClearColor[3] = a; };
		void BeginFrame();
		void EndFrame();

		void RenderToTexture(); // Render scene to the texture
		ID3D11ShaderResourceView* GetShaderResourceView() const {return  pShaderResourceView.Get();	} // For ImGui::Image
		void ResizeRenderTarget(int width, int height);

		void SetViewMatrix(XMFLOAT4X4 viewmatrix) { viewMatrix = viewmatrix; }
		void SetProjectionMatrix(XMFLOAT4X4 projMatrix) { projectionMatrix = projMatrix; }

		UINT GetCubeCount() const { return cubeCount; }
		void RenderCube(UINT NumberOfCubes);

		UINT GetSphereCount() const { return sphereCount; }
		void RenderSphere(UINT NumberOfSpheres);

		UINT GetPlaneCount() const { return planeCount; }
		void RenderPlane(UINT NumberOfPlanes);

		void CleanUp();

	private:
		HRESULT InitDeviceAndSwapChain();
		HRESULT InitRenderTargetView();
		HRESULT InitViewPort();
		HRESULT InitShadersAndInputLayout();
		HRESULT InitSamplerState();
		HRESULT InitDepthStencil();
		HRESULT InitRasterizer();
		HRESULT InitImGui();
		HRESULT InitConstantBuffer();
		HRESULT InitCube();
		HRESULT InitSphere();
		HRESULT InitPlane();

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

		UINT mSampleCount = 4;
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
		UINT cubeCount = 0;

		// Sphere
		ComPtr<ID3D11Buffer> pSphereVertexBuffer;
		ComPtr<ID3D11Buffer> pSphereIndexBuffer;
		std::vector<Vertex> sphereVertices;
		std::vector<WORD> sphereIndices;
		UINT sphereCount = 0;

		// Plane
		ComPtr<ID3D11Buffer> pPlaneVertexBuffer;
		ComPtr<ID3D11Buffer> pPlaneIndexBuffer;
		UINT planeCount = 0;

		// Primitive Object Lists
		std::vector<BlackJawz::GameObject::GameObject*> pGameObjectListCube;
		std::vector<BlackJawz::GameObject::GameObject*> pGameObjectListSphere;
		std::vector<BlackJawz::GameObject::GameObject*> pGameObjectListPlane;

		// Constant Buffer
		ComPtr<ID3D11Buffer> pConstantBuffer;

		// Camera
		XMFLOAT4X4 viewMatrix;
		XMFLOAT4X4 projectionMatrix;

		XMFLOAT4X4 mWorld;
	};
}