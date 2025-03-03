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

		bool HasComponent(BlackJawz::Entity::Entity entity) const
		{
			return transformArray.HasData(entity);
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

		bool HasComponent(BlackJawz::Entity::Entity entity) const
		{
			return appearanceArray.HasData(entity);
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

	class LightSystem : public System
	{
	private:
		// Reference to the Appearance component array
		BlackJawz::Component::ComponentArray<BlackJawz::Component::Light>& lightArray;
		BlackJawz::Component::ComponentArray<BlackJawz::Component::Transform>& transformArray;

	public:
		// Constructor where appearanceArray is passed in
		LightSystem(BlackJawz::Component::ComponentArray<BlackJawz::Component::Light>& lArray, 
			BlackJawz::Component::ComponentArray<BlackJawz::Component::Transform>& tArray)
			: lightArray(lArray), transformArray(tArray) {
		}

		BlackJawz::Component::Light& GetLight(BlackJawz::Entity::Entity entity)
		{
			return lightArray.GetData(entity);
		}

		const std::set<BlackJawz::Entity::Entity>& GetEntities() const
		{
			return entities;
		}

		void Update()
		{
			for (auto entity : entities)
			{
				auto& light = lightArray.GetData(entity);
				auto& transform = transformArray.GetData(entity);

				if (light.Type == BlackJawz::Component::LightType::Directional)
				{
					// Use direction only
					light.Direction = transform.rotation; // Assuming rotation holds direction
				}
				else
				{
					// Use position from Transform for Point & Spot Lights
					XMFLOAT3 lightPos = transform.position;

					// Send to shader
				//	shader.SetLightPosition(entity, lightPos);
				}
			}
		}

		bool HasComponent(BlackJawz::Entity::Entity entity) const
		{
			return lightArray.HasData(entity);
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
