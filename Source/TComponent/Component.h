#pragma once


#include <typeinfo>
#include <typeindex>
#include <vector>
#include <memory>
#include <iostream>
#include <assert.h>
#include <string>
#include<stack>
namespace tEngine {
	template<typename T> class ComponentManager;
	/// <summary>
	/// Remember this class use Deep Copy
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<class T>
	class ComponentInstance final {
		friend class ComponentManager<T>;
	public:
		ComponentInstance(uint32_t id, ComponentManager<T>* pool) :id(id), pool(pool) {};
		~ComponentInstance();
		void operator=(const ComponentInstance& rhs);
		void operator=(const T& rhs);
		const auto& getComponent()const;
		auto& getComponent();
		ComponentInstance(const ComponentInstance& rhs);
	private:
		//bool exist = true;
		const uint32_t id;
		ComponentManager<T>* pool;
	};
	template<typename exampleComponent>
	class ComponentManager final {
	public:
		using Handle = uint32_t;
		using c_instance = ComponentInstance<exampleComponent>;
		friend class ComponentInstance<exampleComponent>;
		template<class ...Args>
		c_instance createComponent(Args... args) {
			auto handle = createComponentId(args...);
			return ComponentInstance<exampleComponent>(handle, this);
		}
		void removeComponent(Handle id) {
			//ensure it's alive
			bool alive = false;
			for (auto iter = alivePool.begin(); iter != alivePool.end(); ++iter) {
				if ((*iter) == id) {
					alivePool.erase(iter); 
					alive = true;
					break;
				}
			}
			assert(alive && "cannot remove an invalid component");
			if (id == componentsMap.size()-1) {
				componentsMap.pop_back();
			}
			else {
				vHandlePool.push(id);
			}
		}
		void removeComponent(const c_instance& ins) {
			removeComponent(ins.id);
		}
		const exampleComponent& getComponent(Handle id)const {
			return componentsMap[id];
		}
		exampleComponent& getComponent(Handle id) {
			return componentsMap[id];
		}
	private:
		/// <summary>
		/// Avoid constructing ComponentInstance, instead constructing id
		/// </summary>
		/// <typeparam name="...Args"></typeparam>
		/// <param name="...args"></param>
		/// <returns></returns>
		template<class ...Args>
		Handle createComponentId(Args... args) {
			if (vHandlePool.size() == 0) {
				componentsMap.emplace_back(exampleComponent(args...));
				auto handle = componentsMap.size() - 1;
				alivePool.push_back(handle);
				return handle;

			}
			auto handle = vHandlePool.top();
			vHandlePool.pop();
			new(&componentsMap[handle])exampleComponent(args...);
			alivePool.push_back(handle);
			return handle;
		}
		std::stack<Handle> vHandlePool;
		std::vector<exampleComponent> componentsMap;
		std::vector<Handle> alivePool;
	};
	template<typename T>
	 ComponentInstance<T>::~ComponentInstance() {
		pool->removeComponent(id);
	}
	template<typename T>
	void ComponentInstance<T>::operator=(const ComponentInstance<T>& rhs) {
		pool->getComponent(id) = rhs.pool->getComponent(rhs.id);
	}
	template<typename T>
	void ComponentInstance<T>::operator=(const T& rhs) {
		getComponent() = rhs;
	}
	template<typename T>
	const auto& ComponentInstance<T>::getComponent()const {
		return pool->getComponent(id);
	}
	template<typename T>
	auto& ComponentInstance<T>::getComponent() {
		return pool->getComponent(id);
	}

	template<typename T>
	ComponentInstance<T>::ComponentInstance<T>(const ComponentInstance<T>& rhs) :
		id(rhs.pool->createComponentId(rhs.getComponent())), pool(rhs.pool) {
	}

	
}