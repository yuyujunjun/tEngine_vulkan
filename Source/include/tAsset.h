#pragma once
#include"SimpleGeometry.h"
#include"tResource.h"
#include<string>
#include<atomic>
namespace tEngine {
	struct Asset {
	public:
		Asset();
		uint32_t GetID() { return id; }
	private:
		static std::atomic<uint32_t> ID;
		uint32_t id;
	};
	struct MeshAsset :public Asset {
		Mesh mesh;
	};
	struct ImageAsset :public Asset {
		
	};
}