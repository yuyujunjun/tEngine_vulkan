#pragma once
#include<atomic>
#include<typeinfo>
#include<memory>
#include<vector>
#include<unordered_map>
//#include<atomic>
#include"tTransform.h"
#include"Component.h"
namespace tEngine {
	class Component;
	class GameObject_;
	using GameObject = std::shared_ptr<GameObject_>;

	class GameObject_ {
	public:
		static GameObject Create() {
			static std::atomic<uint32_t> id = 0;
			uint32_t x=id.fetch_add(1);
			return std::make_shared<GameObject_>(x);
			
		}
		GameObject_(uint32_t id) :id(id) {};
		template<typename T,typename ...Args>
		T* AddComponent(Args... args) {
			components.emplace_back(new T(this,args...));
			auto h_id = typeid(T).hash_code();
			component_id.emplace_back(h_id);
			//componentExist[typeid(T).hash_code()].
			if (componentExist.count(h_id) == 0) {
				componentExist.insert({ h_id,0 });
			}
			else {
				componentExist[h_id] += 1;
			}
			return static_cast<T*>(components.back());
		}
		template<typename T>
		void RemoveComponent() {
			auto idx = typeid(T).hash_code();
			for (unsigned i = component_id.size()-1; i >=0;--i) {
				if (idx == component_id[i]) {
					auto p = components[i];
					components[i] = nullptr;
					component_id.erase(component_id.begin() + i);
					components.erase(component_id.begin() + i);
					componentExist[idx]-=1;
					if (componentExist[idx] == 0)componentExist.erase(idx);
					delete p;
					
				}
			}
		}
		template<typename T>
		T* getComponent() {
			auto idx = typeid(T).hash_code();
			for (int i = component_id.size() - 1; i >= 0; --i) {
				if (idx == component_id[i]) {
					return static_cast<T*>(components[i]);
				}
			}
			return 0;
		}
		template<typename T>
		const T* getComponent()const {
			auto idx = typeid(T).hash_code();
			for (unsigned i = component_id.size() - 1; i >= 0; --i) {
				if (idx == component_id[i]) {
					return static_cast<T*>(components[i]);
				}
			}
			return 0;
		}
		template<typename T>
		bool hasComponent()const {
			return componentExist.count(typeid(T).hash_code()) != 0;
		}
		void clearComponents() {
			for (auto& c : components) {
				delete c;
			}
			components.clear();
			component_id.clear();
			componentExist.clear();
		}

		~GameObject_() {
			clearComponents();
		}
		uint32_t Identity()const { return id; }
		Transform transform;
		std::vector<Component*> components;
		std::vector<size_t> component_id;
		std::unordered_map<size_t,unsigned> componentExist;
	private:
		uint32_t id;
	};
	inline GameObject createGameObject() {
		return GameObject_::Create();
	}
}