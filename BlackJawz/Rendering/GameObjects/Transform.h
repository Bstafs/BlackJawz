#pragma once
#include "../../pch.h"

namespace BlackJawz::GameObject::Transform
{
	class Transform
	{
	public:
		Transform();
		~Transform();

		void UpdateObjectTransform();

		void SetPosition(XMFLOAT3 position) { objectPosition = position; }
		void SetPosition(float x, float y, float z) { objectPosition.x = x; objectPosition.y = y; objectPosition.z = z; }
		XMFLOAT3 GetPosition() const { return objectPosition; }

		void SetScale(XMFLOAT3 scale) { objectScale = scale; }
		void SetScale(float x, float y, float z) { objectScale.x = x; objectScale.y = y; objectScale.z = z; }
		XMFLOAT3 GetScale() const { return objectScale; }

		void SetRotation(XMFLOAT3 rotation) { objectRotation = rotation; }
		void SetRotation(float x, float y, float z) { objectRotation.x = x; objectRotation.y = y; objectRotation.z = z; }
		XMFLOAT3 GetRotation() const { return objectRotation; }

		XMMATRIX GetWorldMatrix() const { return XMLoadFloat4x4(&objectWorld); }
		XMFLOAT4X4 GetWorld() const { return objectWorld; }

	private:
		XMFLOAT3 objectPosition;
		XMFLOAT3 objectScale;
		XMFLOAT3 objectRotation;

		XMFLOAT4X4 objectWorld;
	};
}