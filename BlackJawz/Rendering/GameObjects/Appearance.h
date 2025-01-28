#pragma once
#include "../../pch.h"

namespace BlackJawz::GameObject::Appearance
{
	struct Geometry
	{
		ComPtr<ID3D11Buffer> pVertexBuffer;
		ComPtr<ID3D11Buffer> pIndexBuffer;
		UINT IndicesCount;

		UINT vertexBufferStride;
		UINT vertexBufferOffset;
	};

	class Appearance
	{
	public:
		Appearance(Geometry geo);
		~Appearance();

		Geometry GetGeometry() const { return objectGeometry; }

		void Draw(ID3D11DeviceContext* pImmediateContext);

	private:
		Geometry objectGeometry;
	};
}
