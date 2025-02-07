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
		XMFLOAT3 GetPosition() const { return position; }
		XMFLOAT3 GetRotation() const { return rotation; }

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

	enum class LightType : int
	{
		Point = 0,
		Directional = 1,
		Spot = 2
	};

	struct Light
	{
		Light() = default;

		Light(LightType type) : Type(type) // Constructor to initialize based on type
		{
			switch (Type)
			{
			case LightType::Point:
				DiffuseLight = { 1.0f, 0.8f, 0.6f, 1.0f };
				AmbientLight = { 0.1f, 0.1f, 0.1f, 1.0f };
				SpecularLight = { 1.0f, 0.9f, 0.7f, 1.0f };
				SpecularPower = 32.0f;
				Range = 15.0f;
				Attenuation = { 1.0f, 0.1f, 0.01f }; // Default attenuation
				Intensity = 1.0f;
				Direction = { 0.0f, 0.0f, 0.0f };
				SpotInnerCone = 0.0f;
				SpotOuterCone = 0.0f;
				break;

			case LightType::Directional:
				DiffuseLight = { 1.0f, 1.0f, 1.0f, 1.0f };
				AmbientLight = { 0.2f, 0.2f, 0.2f, 1.0f };
				SpecularLight = { 1.0f, 1.0f, 1.0f, 1.0f };
				SpecularPower = 64.0f;
				Direction = { -0.5f, -1.0f, -0.5f }; // Sunlight direction
				Intensity = 1.0f;
				Range = 0.0f;
				Attenuation = { 0.0f, 0.0f, 0.0f };
				SpotInnerCone = 0.0f;
				SpotOuterCone = 0.0f;
				break;

			case LightType::Spot:
				DiffuseLight = { 1.0f, 1.0f, 0.9f, 1.0f };
				AmbientLight = { 0.1f, 0.1f, 0.1f, 1.0f };
				SpecularLight = { 1.0f, 1.0f, 1.0f, 1.0f };
				SpecularPower = 32.0f;
				Range = 20.0f;
				Attenuation = { 1.0f, 0.1f, 0.02f };
				Direction = { 0.0f, -1.0f, 0.0f }; // Spot pointing down
				SpotInnerCone = cos(XMConvertToRadians(15.0f));
				SpotOuterCone = cos(XMConvertToRadians(30.0f));
				Intensity = 1.5f;
				break;
			}
		}

		LightType Type = LightType::Point;

		XMFLOAT4 DiffuseLight;
		XMFLOAT4 AmbientLight;
		XMFLOAT4 SpecularLight;
		float SpecularPower;

		float Range; // For Point & Spot Lights
		XMFLOAT3 Direction;
		float Intensity;

		XMFLOAT3 Attenuation;
		float Padding;

		float SpotInnerCone; // For Spotlights
		float SpotOuterCone;
	};
}
