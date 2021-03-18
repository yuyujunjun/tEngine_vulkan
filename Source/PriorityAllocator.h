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
		
		std::shared_ptr<C> request(const Attribute& value) {
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
		//move last element to front and return it
		std::shared_ptr<C>& moveLastToFront(const Attribute& value) {
			assert(order.size() != 0);
			uint32_t idx = order.back();
			order.pop_back();
			order.emplace_front(idx);
			attribute[idx] = value;
			return objectPool[idx];
		}
		bool isFull() {
			return objectPool.size() >= RingSize;
		}
		//return new C* 
		std::shared_ptr<C>& allocate(const Attribute& att) {
			if (objectPool.size() < RingSize) {
				objectPool.emplace_back(std::make_shared<C>());
				attribute.emplace_back(att);
				order.emplace_front(objectPool.size() - 1);
				return  objectPool.back() ;
			}
			else {
				assert(order.size() != 0);
				uint32_t idx = order.back();
				order.pop_back();
				order.emplace_front(idx);
				attribute[idx] = att;
				objectPool[idx] = std::make_shared<C>();
				return objectPool[idx];
			}

		}
		const Attribute& getFirstAttribute() {
			return attribute[order.begin()];
		}
		template<typename ...P>
		std::shared_ptr<C>& allocate(const Attribute& att,P... args) {
			if (objectPool.size() < RingSize) {
				auto address = std::make_shared<C>(args...);
				objectPool.emplace_back(address);
				attribute.emplace_back(att);
				order.emplace_front(objectPool.size() - 1);
				auto& t = objectPool.back();
				return objectPool.back();
			}
			else {
				assert(order.size()!=0);
				uint32_t idx = order.back();
				order.pop_back();
				order.emplace_front(idx);
				attribute[idx] = att;
				objectPool[idx]=std::make_shared<C>(args...);
				return objectPool[idx];
			}
			
		}
		~RingPool() {
			clear();
		}
		void clear() {
			objectPool.clear();
			order.clear();
			attribute.clear();
		}
	private:
		
	//	std::function<bool(const T&,const T&)> compare;
		std::list<uint32_t> order;
		std::vector<Attribute> attribute;
		std::vector<std::shared_ptr<C>> objectPool;
	};
}