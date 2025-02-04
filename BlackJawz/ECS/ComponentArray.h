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
            size_t index = size;
            entityToIndex[entity] = index;
            indexToEntity[index] = entity;
            componentArray[index] = component;
            ++size;
        }

        void RemoveData(BlackJawz::Entity::Entity entity)
        {
            size_t index = entityToIndex[entity];
            entityToIndex.erase(entity);
            indexToEntity.erase(index);
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
