#pragma once
#include "../pch.h"
#include "EntityManager.h"

namespace BlackJawz::Component
{


	class IComponentArray
	{
	public:
		virtual ~IComponentArray() = default;
		virtual void EntityDestroyed(BlackJawz::Entity::Entity entity) = 0;
	};

    template<typename T>
    class ComponentArray : public IComponentArray 
    {
    private:
        std::array<T, BlackJawz::Entity::MAX_ENTITIES> componentArray;
        std::unordered_map<BlackJawz::Entity::Entity, size_t> entityToIndex;
        std::unordered_map<size_t, BlackJawz::Entity::Entity> indexToEntity;
        size_t size = 0;

    public:
        void InsertData(BlackJawz::Entity::Entity entity, T component)
        {
            if (HasData(entity))  // Prevent overwriting an existing entity
            {
                componentArray[entityToIndex[entity]] = component;
                return;
            }

            size_t index = size;
            entityToIndex[entity] = index;
            indexToEntity[index] = entity;
            componentArray[index] = component;
            ++size;
        }

        void RemoveData(BlackJawz::Entity::Entity entity)
        {
            // Ensure the entity exists
            auto it = entityToIndex.find(entity);
            if (it == entityToIndex.end())
                return; // Entity doesn't exist, avoid crash

            size_t index = it->second;
            size_t lastIndex = size - 1;

            if (index != lastIndex)
            {
                // Move last element into the deleted entity's slot
                componentArray[index] = componentArray[lastIndex];

                BlackJawz::Entity::Entity lastEntity = indexToEntity[lastIndex];
                entityToIndex[lastEntity] = index;  // Update moved entity's index
                indexToEntity[index] = lastEntity;
            }

            // Erase entity references
            entityToIndex.erase(entity);
            indexToEntity.erase(index); // Fix: Erase `index`, not `lastIndex`

            --size;
        }

        T& GetData(BlackJawz::Entity::Entity entity)
        {
            return componentArray[entityToIndex[entity]];
        }

        bool HasData(BlackJawz::Entity::Entity entity) const
        {
            return entityToIndex.find(entity) != entityToIndex.end();
        }

        void EntityDestroyed(BlackJawz::Entity::Entity entity) override
        {
            if (entityToIndex.find(entity) != entityToIndex.end()) 
            {
                RemoveData(entity);
            }
        }
    };
}
