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
		BlackJawz::Component::ComponentArray<BlackJawz::Component::Transform>& transformArray;

	public:
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

		void AddEntity(BlackJawz::Entity::Entity entity)
		{
			entities.insert(entity);
		}

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
					light.Direction = transform.rotation; 
				}
				else
				{
					XMFLOAT3 lightPos = transform.position;
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
