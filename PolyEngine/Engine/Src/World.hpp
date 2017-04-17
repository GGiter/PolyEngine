#pragma once

#include <Core.hpp>

#include "Entity.hpp"

#include <unordered_map>

namespace Poly {

	struct InputState;

	class ENGINE_DLLEXPORT World : public BaseObject<>
	{
	public:
		World();
		virtual ~World();

		//TODO implement world
		UniqueID SpawnEntity();
		void DestroyEntity(const UniqueID& entityId);

		//------------------------------------------------------------------------------
		template<typename T, typename... Args>
		void AddComponent(const UniqueID& entityId, Args&&... args)
		{
			T* ptr = GetComponentAllocator<T>()->Alloc();
			::new(ptr)(std::forward<Args>(args)...);
			Entity* ent = IDToEntityMap[entityId];
			HEAVY_ASSERTE(ent, "Invalid entity ID");
			HEAVY_ASSERTE(ent->HasComponent(GET_COMPONENT_ID(T)), "Failed at AddComponent() - a component of a given UniqueID already exists!");
			ent->ComponentPosessionFlags.set(GET_COMPONENT_ID(T), true);
			ent->Components[GET_COMPONENT_ID(T)] = ptr;
			ptr->Owner = ent;
		}

		//------------------------------------------------------------------------------
		template<typename T>
		void RemoveComponent(const UniqueID& entityId)
		{
			Entity* ent = IDToEntityMap[entityId];
			HEAVY_ASSERTE(ent, "Invalid entity ID");
			HEAVY_ASSERTE(!ent->HasComponent(GET_COMPONENT_ID(T)), "Failed at RemoveComponent() - a component of a given UniqueID does not exist!");
			ent->ComponentPosessionFlags.set(GET_COMPONENT_ID(T), false);
			T* component = ent->Components[GET_COMPONENT_ID(T)];
			ent->Components[GET_COMPONENT_ID(T)] = nullptr;
			component->~T();
			GetComponentAllocator<T>()->Free(component);
		}

		//------------------------------------------------------------------------------
		template<typename T>
		IterablePoolAllocator<T>* GetComponentAllocator()
		{
			size_t componentID = GET_COMPONENT_ID(T);
			HEAVY_ASSERTE(componentID < MAX_COMPONENTS_COUNT, "Invalid component ID");
			if (ComponentAllocators[componentID] == nullptr)
				ComponentAllocators[componentID] = new IterablePoolAllocator<T>(MAX_ENTITY_COUNT);
			return static_cast<IterablePoolAllocator<T>*>(ComponentAllocators[componentID]);
		}

		//------------------------------------------------------------------------------
		//////////////////////////////
		/// Gets a component of a specified type and UniqueID.
		/// @tparam T - component type to get
		/// @param entityId - UniqueID of a component to get
		/// @return pointer to a specified component or a nullptr, if none was found
		/// @see AddComponent()
		/// @see RemoveComponent()
		//////////////////////////////
		template<typename T>
		T* GetComponent(const UniqueID& entityId)
		{
			HEAVY_ASSERTE(entityId < MAX_COMPONENTS_COUNT, "Invalid component ID");
			auto iter = IDToEntityMap.find(entityId);
			HEAVY_ASSERTE(iter != IDToEntityMap.end(), "Invalid entityId - entity with that ID does not exist!");
			return iter->second->GetComponent<T>();
		}

	private:
		std::unordered_map<UniqueID, Entity*> IDToEntityMap;

		void RemoveComponentById(Entity* ent, size_t id);

		// Allocators
		PoolAllocator<Entity> EntitiesAllocator;
		IterablePoolAllocatorBase* ComponentAllocators[MAX_COMPONENTS_COUNT];
	};

}