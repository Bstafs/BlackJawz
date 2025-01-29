#pragma once
#include "../pch.h"

namespace BlackJawz::Entity
{
	using Entity = std::uint32_t;
	const Entity MAX_ENTITIES = 5000;

	class EntityManager
	{
	public:
		EntityManager()
		{
			for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) 
			{
				availableEntities.push(entity);
			}
		}

		Entity CreateEntity()
		{
			assert(livingEntityCount < MAX_ENTITIES && "Too many entities in existence.");
			Entity id = availableEntities.front();
			availableEntities.pop();
			++livingEntityCount;
			return id;
		}

		void DestroyEntity(Entity entity)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");
			availableEntities.push(entity);
			entitySignatures[entity].reset();
			--livingEntityCount;
		}

		void SetSignature(Entity entity, std::bitset<32> signature) { entitySignatures[entity] = signature; }
		std::bitset<32> GetSignature(Entity entity) {return entitySignatures[entity]; }

	private:
		std::queue<Entity> availableEntities;

		std::array<std::bitset<32>, MAX_ENTITIES> entitySignatures;

		uint32_t livingEntityCount = 0;
	};
}