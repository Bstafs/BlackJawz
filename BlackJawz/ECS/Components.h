#pragma once
#include "../pch.h"

namespace BlackJawz::Component
{
	struct Geometry
	{
		ComPtr<ID3D11Buffer> pVertexBuffer;
		ComPtr<ID3D11Buffer> pIndexBuffer;
		UINT IndicesCount;

		UINT vertexBufferStride;
		UINT vertexBufferOffset;
	};

	struct Transform
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 rotation;
		DirectX::XMFLOAT3 scale;
		DirectX::XMFLOAT4X4 worldMatrix;

		Transform()
			: position(0.0f, 0.0f, 0.0f),
			rotation(0.0f, 0.0f, 0.0f),
			scale(1.0f, 1.0f, 1.0f)
		{
			XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
		}

		XMMATRIX GetWorldMatrix() const { return XMLoadFloat4x4(&worldMatrix); }

		void UpdateWorldMatrix()
		{
			DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
			DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationX(rotation.x) * DirectX::XMMatrixRotationY(rotation.y) * DirectX::XMMatrixRotationZ(rotation.z);
			DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

			DirectX::XMStoreFloat4x4(&worldMatrix, scaleMatrix * rotationMatrix * translationMatrix);
		}
	};

	struct Appearance
	{
		// Constructor to initialize with Geometry data
		Appearance() = default;

		Appearance(Geometry geo, ID3D11ShaderResourceView* texture) : objectGeometry(geo), textureData(texture) {}

		// Getter to access the Geometry data
		Geometry GetGeometry() const { return objectGeometry; }
		Geometry objectGeometry;  // Store the geometry data

		ComPtr<ID3D11ShaderResourceView> GetTexture() const { return textureData; }
		bool HasTexture()
		{
			if (textureData != nullptr)
			{
				return true;
			}
		}

		ComPtr<ID3D11ShaderResourceView> textureData;
	};

	enum class LightType
	{
		Point,
		Directional,
		Spot
	};

	struct Light
	{
		Light() = default;

		LightType Type = LightType::Point; // Default to Point Light

		XMFLOAT4 DiffuseLight = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT4 AmbientLight = { 0.2f, 0.2f, 0.2f, 1.0f };
		XMFLOAT4 SpecularLight = { 1.0f, 1.0f, 1.0f, 1.0f };

		float SpecularPower = 32.0f;

		// For Point & Spot Lights
		float Range = 10.0f;

		// For Directional & Spot Lights
		XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f };
		float Intensity = 1.0f;

		// Attenuation for Point & Spot Lights
		XMFLOAT3 Attenuation = { 1.0f, 0.1f, 0.01f }; // Constant, Linear, Quadratic
		float Padding = 0.0f; // Padding for memory alignment

		// Spotlight-specific
		float SpotInnerCone = 0.8f; // Inner cone (cosine of angle)
		float SpotOuterCone = 0.5f; // Outer cone (cosine of angle)
	};
}
