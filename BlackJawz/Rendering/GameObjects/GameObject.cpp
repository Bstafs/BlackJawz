#include "GameObject.h"

BlackJawz::GameObject::GameObject::GameObject(std::string name, BlackJawz::GameObject::Appearance::Geometry objectGeometry)
{
	objectName = name;
	pGameObjectTransform = new BlackJawz::GameObject::Transform::Transform();
	pGameObjectAppearance = new BlackJawz::GameObject::Appearance::Appearance(objectGeometry);
}

BlackJawz::GameObject::GameObject::~GameObject()
{
	
}

void BlackJawz::GameObject::GameObject::Update()
{
	pGameObjectTransform->UpdateObjectTransform();
}

void BlackJawz::GameObject::GameObject::Draw(ID3D11DeviceContext* pImmediateContext)
{
	pGameObjectAppearance->Draw(pImmediateContext);
}