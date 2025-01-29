#pragma once
#include "../pch.h"
#include "EntityManager.h"

namespace BlackJawz::System
{
	class System 
	{
	public:
		std::set<BlackJawz::Entity::Entity> entities;
	};

    class SystemManager {
    private:
        std::unordered_map<const char*, std::shared_ptr<System>> systems;
        std::unordered_map<const char*, std::bitset<32>> signatures;

    public:
        template<typename T>
        std::shared_ptr<T> RegisterSystem() 
        {
            const char* typeName = typeid(T).name();
            auto system = std::make_shared<T>();
            systems[typeName] = system;
            return system;
        }

        template<typename T>
        void SetSignature(std::bitset<32> signature) 
        {
            const char* typeName = typeid(T).name();
            signatures[typeName] = signature;
        }

        void EntityDestroyed(BlackJawz::Entity::Entity entity)
        {
            for (auto& [name, system] : systems)
            {
                system->entities.erase(entity);
            }
        }
    };

}

