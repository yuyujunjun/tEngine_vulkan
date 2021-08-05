#pragma once
#include<bitset>
#include<vector>
#include<unordered_map>
#include<assert.h>
#include<queue>
namespace tEngine {
	int s_componentCounter = 1;
	template<class T>
	int GetTypeId() {
		static int s_componentId = s_componentCounter++;
		return s_componentId;
	}
	using   EntityID = uint32_t;
	const int MAX_COMPONENTS = 64;
	using ComponentMask = std::bitset<MAX_COMPONENTS>;


	class BaseComponentPool {
	public:
		virtual void RemoveComponent(uint32_t entity_id) = 0;
		//	virtual ~BaseComponentPool() = 0;
		virtual bool HasComponent(uint32_t entity_id)const = 0;

	};
	template<typename T>
	class ComponentPool :public BaseComponentPool {

		const uint32_t start_size = 64;
		size_t element_size;
		char* plainMemory = nullptr;
		size_t currentSize = 0;
		uint32_t uniqueID_allocator = 0;
		std::queue<uint32_t> usableId;
		std::unordered_map<uint32_t, std::vector<uint32_t>> entity_to_memoryOffset;

		void addStorage() {

			uint32_t currentIdx = currentSize / element_size;
			for (unsigned i = currentIdx; i < currentIdx + start_size; ++i) {
				usableId.push(i);
			}
			auto currentMemory = plainMemory;
			plainMemory = (char*)malloc(currentSize + start_size * element_size);
			if (currentMemory != nullptr) {
				memcpy(plainMemory, currentMemory, currentSize);
				free(currentMemory);
			}
			currentSize += start_size * element_size;

		}
		uint32_t requestMemory() {
			if (usableId.size() == 0) {
				addStorage();
			}
			uint32_t memoryOffset = usableId.front();
			usableId.pop();
			return memoryOffset;
		}
		void* getMemory(uint32_t memoryOffset) {
			return &(plainMemory[element_size * memoryOffset]);
		}
		T* GetComponentAddress(uint32_t componentPosition) {
			void* address = (&plainMemory[componentPosition * element_size]);
			return static_cast<T*>(address);
		}

	public:
		friend class GameObject_;
		ComponentPool() :uniqueID_allocator(0), element_size(sizeof(T)) {}
		template<typename ...Args>
		T* AddComponent(EntityID index, Args... args) {
			//check if type exist
			auto memoryOffset = requestMemory();
			new (getMemory(memoryOffset))T(args...);
			entity_to_memoryOffset[index].push_back(memoryOffset);
			return GetComponentAddress(memoryOffset);
		}
		T* GetComponentFirst(EntityID index) {
			return GetComponentAddress(entity_to_memoryOffset.at(index)[0]);
		}
		T* GetComponentLast(EntityID index) {
			return GetComponentAddress(entity_to_memoryOffset.at(index).back());
		}
		std::vector<T*> GetComponents(EntityID index) {
			std::vector<T*> v;
			const auto& memoryOffsets = entity_to_memoryOffset.at(index);
			for (auto offset : memoryOffsets) {
				v.push_back(GetComponentAddress(offset));
			}
			return v;
		}
		//make sure that the component has not be removed
		bool HasComponent(uint32_t entityId)const override {
			return (entity_to_memoryOffset.count(entityId) != 0);
		}
		void RemoveComponent(uint32_t entityId)override {
			const auto& memoryOffsets = entity_to_memoryOffset.at(entityId);
			for (auto offset : memoryOffsets) {
				usableId.push(offset);
				T* address = (T*)getMemory(offset);
				address->~T();
			}
			entity_to_memoryOffset.erase(entityId);
		}
		virtual ~ComponentPool() {
			for (auto& p : entity_to_memoryOffset) {
				for (auto offset : p.second) {
					T* address = (T*)getMemory(offset);
					address->~T();
				}

			}
			free(plainMemory);
			entity_to_memoryOffset.clear();
		}
	};






	struct EcsManager {
		struct EntityDesc {
			EntityID id;
			ComponentMask mask;
		};

		EntityID NewEntity() {
			static EntityID id = 0;
			if (id_to_pos.size() <= id) {
				id_to_pos.resize(id + 1);
			}
			if (freeEntities.size() > 0) {
				size_t entity_pos = freeEntities.front();
				freeEntities.pop();

				id_to_pos[id] = entity_pos;
				entities[entity_pos] = { id,ComponentMask() };
			}
			else {
				id_to_pos[id] = entities.size();
				entities.push_back({ id,ComponentMask() });
			}
			return id++;
		}
		size_t GetEntityPos(EntityID id) {
			auto entity_pos = id_to_pos[id];
			assert(entities[entity_pos].id == id);
			/*if (entities[entity_pos].id != id)
				return -1;*/
			return entity_pos;
		}
		bool IsPosHasEntity(int id) {
			return entities[id].id != -1;
		}
		bool IsIDValid(EntityID id) {
			return id_to_pos[id] != -1;
		}
		template<typename T, typename ...Args>
		T* AddComponent(EntityID id, Args...args) {
			int componentId = GetTypeId<T>();
			if(entities[GetEntityPos(id)].mask.test(componentId))
				return GetComponent<T>(id);
			if (componentPools.size() <= componentId) {
				componentPools.resize(componentId + 1, nullptr);
			}
			if (componentPools[componentId] == nullptr) {
				componentPools[componentId] = new ComponentPool<T>();
			}
			T* pComponent = static_cast<ComponentPool<T>*>(componentPools[componentId])->AddComponent(id, args...);

			entities[GetEntityPos(id)].mask.set(componentId);
			return pComponent;
		}
		template<typename T>
		T* GetComponent(EntityID id) {
			int componentId = GetTypeId<T>();
			if (!entities[GetEntityPos(id)].mask.test(componentId)) {
				return nullptr;
			}
			T* pComponent = static_cast<ComponentPool<T>*>(componentPools[componentId])->GetComponentFirst(id);
			return pComponent;
		}
		template<typename T>
		std::vector<T*> GetComponents(EntityID index) {
			int componentId = GetTypeId<T>();
			if (!entities[GetEntityPos(index)].mask.test(componentId)) {
				return { nullptr };
			}
			return  static_cast<ComponentPool<T>*>(componentPools[componentId])->GetComponents(index);
		}
		template<typename T>
		void RemoveComponent(EntityID id) {
			int componentId = GetTypeId<T>();
			auto entity_pos = GetEntityPos(id);
			entities[entity_pos].mask.reset(componentId);
			static_cast<ComponentPool<T>*>(componentPools[componentId])->RemoveComponent(id);
		}
		void DestroyEntity(EntityID id) {
			auto entity_pos = GetEntityPos(id);
			auto& desc = entities[entity_pos];
			for (int i = 0; i < desc.mask.size(); ++i) {
				if (desc.mask.test(i)) {
					componentPools[i]->RemoveComponent(id);
					desc.mask.reset(i);
					if (desc.mask.none())break;
				}
			}
			desc.mask.reset();
			desc.id = -1;
			id_to_pos[id] = -1;
			freeEntities.push(entity_pos);
		}
		std::vector<size_t> id_to_pos;
		std::vector<EntityDesc> entities;
		std::queue<uint32_t> freeEntities;
		std::vector<BaseComponentPool*> componentPools;
	};
	template<typename ...ComponentTypes>
	struct SceneView {
		SceneView(EcsManager& ecs) :ecsSystem(&ecs) {
			if (sizeof...(ComponentTypes) == 0) {
				all = true;
			}
			else {
				int componentIds[] = { 0,GetId<ComponentTypes>()... };
				for (unsigned i = 0; i < sizeof...(ComponentTypes) + 1; ++i) {
					componentMask.set(componentIds[i]);
				}
			}

		};
		struct Iterator {
			Iterator(EcsManager* ecs, int index, ComponentMask mask, bool all) :ecsSystem(ecs), index(index), mask(mask), all(all) {}
			EntityID operator*()const {
				return ecsSystem->entities[index].id;
			}
			bool operator==(const Iterator& other)const {
				return index == other.index || index == ecsSystem->entities.size();
			}
			bool operator!=(const Iterator& other) const
			{
				return index != other.index && index != ecsSystem->entities.size();
			}
			bool ValidIndex(int index) {
				return ecsSystem->IsPosHasEntity(index) && (all || (ecsSystem->entities[index].mask & mask) == mask);
			}
			Iterator& operator++()
			{
				do
				{
					index++;
				} while (index < ecsSystem->entities.size() && !ValidIndex(index));
				return *this;
			}
			int index;
			EcsManager* ecsSystem;
			ComponentMask mask;
			bool all{ false };
		};
		const Iterator begin() const
		{
			int firstIndex = 0;
			while (firstIndex < ecsSystem->entities.size() &&
				(!ecsSystem->IsPosHasEntity(firstIndex) || componentMask != (componentMask & ecsSystem->entities[firstIndex].mask)
					))
			{
				firstIndex++;
			}
			return Iterator(ecsSystem, firstIndex, componentMask, all);
		}

		const Iterator end() const
		{
			return Iterator(ecsSystem, ecsSystem->entities.size(), componentMask, all);
			// Give an iterator to the end of this view 
		}
		ComponentMask componentMask;
		EcsManager* ecsSystem;
		bool all{ false };
	};
	class System {
		friend class tWorld;
	public:
		System() = default;
		System(EcsManager* manager) :ecsManager(manager) {}
		EcsManager* ecsManager;
		//virtual void ExecuteAllComponents(float dt) = 0;
	};
}