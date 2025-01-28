#pragma once
#include "../../pch.h"
#include "Transform.h"
#include "Appearance.h"

namespace BlackJawz::GameObject
{
	class GameObject
	{
	public:
		GameObject(BlackJawz::GameObject::Appearance::Geometry objectGeometry);
		~GameObject();

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
