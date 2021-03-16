#pragma once
#include<list>
#include<vector>
#include<functional>
namespace tEngine {
	template <typename C,typename Attribute=C,unsigned RingSize=8>
	class RingPool {
	public:
		RingPool() {
		}
		C* request(const Attribute& value) {
			auto iter = std::find_if(order.begin(), order.end(), [this,value](const uint32_t& a) {return attribute[a] == value; });
			if (iter != order.end()) {

				uint32_t idx = *iter;
				if (iter != order.begin()) {
					order.erase(iter);
					order.push_front(idx);
				}
				return objectPool[idx];
			}
			else {
				return nullptr;
			}
		}
		//return new C* and previous attribute
		C* allocate(const Attribute& att) {
			if (objectPool.size() < RingSize) {
				objectPool.emplace_back(new C());
				attribute.emplace_back(att);
				order.emplace_front(objectPool.size() - 1);
				return  objectPool.back() ;
			}
			else {
				auto idx = order.pop_back();
				order.emplace_front(idx);
				attrubute[idx] = att;
				new(objectPool[idx])C();
				return objectPool[idx];
			}

		}
		const Attribute& getFirstAttribute() {
			return attribute[order.begin()];
		}
		template<typename ...P>
		C* allocate(const Attribute& att,P... args) {
			if (objectPool.size() < RingSize) {
				objectPool.emplace_back(new C(args...));
				attribute.emplace_back(att);
				order.emplace_front(objectPool.size() - 1);
				return objectPool.back();
			}
			else {
				auto idx = order.pop_back();
				order.emplace_front(idx);
				attrubute[idx] = att;
				new(objectPool[idx])C(args...);
				return objectPool[idx];
			}
			
		}
		~RingPool() {
			clear();
		}
		void SetCompare(std::function<bool(const Attribute&)>){

		}
	private:
		void clear() {
			for (auto& ring : objectPool) {
				delete ring;
			}
		}
		std::function<bool(const T&,const T&)> compare;
		std::list<uint32_t> order;
		std::vector<Attribute> attribute;
		std::vector<C*> objectPool;
	};
}