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

        Appearance(Geometry geo) : objectGeometry(geo) {}

        // Getter to access the Geometry data
        Geometry GetGeometry() const { return objectGeometry; }

        Geometry objectGeometry;  // Store the geometry data
    };
}
