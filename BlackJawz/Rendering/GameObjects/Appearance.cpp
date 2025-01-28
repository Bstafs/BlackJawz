#include "Appearance.h"

BlackJawz::GameObject::Appearance::Appearance::Appearance(Geometry geo) : objectGeometry(geo) 
{

}

BlackJawz::GameObject::Appearance::Appearance::~Appearance()
{

}

void BlackJawz::GameObject::Appearance::Appearance::Draw(ID3D11DeviceContext* pImmediateContext)
{
	pImmediateContext->IASetVertexBuffers(0, 1, objectGeometry.pVertexBuffer.GetAddressOf(), &objectGeometry.vertexBufferStride, &objectGeometry.vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(objectGeometry.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(objectGeometry.IndicesCount, 0, 0);
}