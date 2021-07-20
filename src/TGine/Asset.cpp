#include"Asset.h"
namespace tEngine {
	std::atomic<uint32_t> Asset::ID;
	Asset::Asset() {
		id = ID.fetch_add(1);
	}
}