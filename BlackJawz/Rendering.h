#pragma once
#include "pch.h"
#include "Application.h"

namespace BlackJawz::Application
{
	class Application;
}


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

		void SetBackGroundColour(float r, float g, float b, float a) {  ClearColor[0] = r, ClearColor[1] = g, ClearColor[2] = b, ClearColor[3] = a; };

	private:
		HRESULT InitDeviceAndSwapChain();
		HRESULT InitRenderTargetView();
		HRESULT InitViewPort();
		HRESULT InitSamplerState();
		HRESULT InitDepthStencil();
		HRESULT InitRasterizer();

	private:
		ComPtr<ID3D11Device> pID3D11Device;
		ComPtr<ID3D11DeviceContext> pImmediateContext;
		ComPtr<IDXGISwapChain> pSwapChain;

		D3D_DRIVER_TYPE         _driverType;
		D3D_FEATURE_LEVEL       _featureLevel;

		UINT mSampleCount = 4;
		float ClearColor[4] = {};

		ComPtr<ID3D11SamplerState> pSamplerLinear;

		ComPtr<ID3D11RenderTargetView> pRenderTargetView;

		ComPtr<ID3D11DepthStencilView> pDepthStencilView;
		ComPtr<ID3D11Texture2D> pDepthStencilBuffer;

		ComPtr<ID3D11DepthStencilState> DSLessEqual;
		ComPtr<ID3D11RasterizerState> RSCullNone;
		ComPtr<ID3D11RasterizerState> CCWcullMode;
		ComPtr<ID3D11RasterizerState> CWcullMode;
	};
}