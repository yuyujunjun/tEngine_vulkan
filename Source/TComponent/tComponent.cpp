#include "tComponent.h"
namespace tEngine {
//	using T = float;
	std::atomic<uint32_t> tComponent::hashId=0;
	tComponent::tComponent() {
		ID=hashId.fetch_add(1);
	}
	
	
}
