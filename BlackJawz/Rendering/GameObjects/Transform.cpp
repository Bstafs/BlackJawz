#include "Transform.h"

BlackJawz::GameObject::Transform::Transform::Transform()
{
	objectPosition = XMFLOAT3();
	objectRotation = XMFLOAT3();
	objectScale = XMFLOAT3(1.0f, 1.0f, 1.0f);
}

BlackJawz::GameObject::Transform::Transform::~Transform()
{

}

void BlackJawz::GameObject::Transform::Transform::UpdateObjectTransform()
{
	XMMATRIX scale = XMMatrixScaling(objectScale.x, objectScale.y, objectScale.z);
	XMMATRIX rotation = XMMatrixRotationX(objectRotation.x) * XMMatrixRotationY(objectRotation.y) * XMMatrixRotationZ(objectRotation.z);
	XMMATRIX translation = XMMatrixTranslation(objectPosition.x, objectPosition.y, objectPosition.z);

	XMStoreFloat4x4(&objectWorld, scale * rotation * translation);
}