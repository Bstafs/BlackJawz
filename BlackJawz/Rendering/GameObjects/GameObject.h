#pragma once
#include "../../pch.h"
#include "Transform.h"
#include "Appearance.h"
#include "../../ECS/EntityManager.h"

namespace BlackJawz::GameObject
{
	class GameObject
	{
	public:
		GameObject(std::string name, BlackJawz::GameObject::Appearance::Geometry objectGeometry);
		~GameObject();

		std::string GetObjectName() const { return objectName; }

		BlackJawz::GameObject::Transform::Transform* GetTransform() const { return pGameObjectTransform; }
		BlackJawz::GameObject::Appearance::Appearance* GetAppearance() const { return pGameObjectAppearance; }

		void Update();
		void Draw(ID3D11DeviceContext* pImmediateContext);
	private:

		std::string objectName;
		BlackJawz::GameObject::Transform::Transform* pGameObjectTransform;
		BlackJawz::GameObject::Appearance::Appearance* pGameObjectAppearance;
	};
}
