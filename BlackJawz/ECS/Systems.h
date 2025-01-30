#pragma once
#include "../pch.h"
#include "ComponentArray.h"
#include "Components.h"
#include "SystemManager.h"

namespace BlackJawz::System
{
	class TransformSystem : public System
	{
	private:
		// Assuming transformArray is an instance of ComponentArray<Transform>
		BlackJawz::Component::ComponentArray<BlackJawz::Component::Transform>& transformArray;

	public:
		// Constructor where transformArray is passed in
		TransformSystem(BlackJawz::Component::ComponentArray<BlackJawz::Component::Transform>& transformArray)
			: transformArray(transformArray) {}

		BlackJawz::Component::Transform& GetTransform(BlackJawz::Entity::Entity entity)
		{
			return transformArray.GetData(entity);
		}

		const std::set<BlackJawz::Entity::Entity>& GetEntities() const
		{
			return entities;
		}

		void Update()
		{
			// Iterate over all entities with a Transform component
			for (auto entity : entities)
			{
				// Access the Transform component for each entity
				auto& transform = transformArray.GetData(entity);
				transform.UpdateWorldMatrix();  // Update the world matrix based on position, rotation, and scale
			}
		}

		// You may want a method to add entities to this system when they have a Transform component
		void AddEntity(BlackJawz::Entity::Entity entity)
		{
			entities.insert(entity);
		}

		// Optionally, a method to remove entities when they are destroyed or no longer have a Transform component
		void RemoveEntity(BlackJawz::Entity::Entity entity)
		{
			entities.erase(entity);
		}
	};

	class AppearanceSystem : public System
	{
	private:
		// Reference to the Appearance component array
		BlackJawz::Component::ComponentArray<BlackJawz::Component::Appearance>& appearanceArray;

	public:
		// Constructor where appearanceArray is passed in
		AppearanceSystem(BlackJawz::Component::ComponentArray<BlackJawz::Component::Appearance>& appearanceArray)
			: appearanceArray(appearanceArray) {}

		BlackJawz::Component::Appearance& GetAppearance(BlackJawz::Entity::Entity entity)
		{
			return appearanceArray.GetData(entity);
		}

		const std::set<BlackJawz::Entity::Entity>& GetEntities() const
		{
			return entities;
		}

		void Draw(ID3D11DeviceContext* pImmediateContext)
		{
			// Iterate over all entities with an Appearance component
			for (auto entity : entities)
			{
				// Access the Appearance component for each entity
				auto& appearance = appearanceArray.GetData(entity);
				BlackJawz::Component::Geometry geo = appearance.GetGeometry();

				// Set up the geometry for drawing
				pImmediateContext->IASetVertexBuffers(0, 1, geo.pVertexBuffer.GetAddressOf(), &geo.vertexBufferStride, &geo.vertexBufferOffset);
				pImmediateContext->IASetIndexBuffer(geo.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

				// Draw the entity
				pImmediateContext->DrawIndexed(geo.IndicesCount, 0, 0);
			}
		}

		// Add an entity to the system
		void AddEntity(BlackJawz::Entity::Entity entity)
		{
			entities.insert(entity);
		}

		// Remove an entity from the system
		void RemoveEntity(BlackJawz::Entity::Entity entity)
		{
			entities.erase(entity);
		}
	};

}
