#pragma once
#include<atomic>
namespace tEngine {
	
	class tComponent
	{
		tComponent();
		uint32_t getID() { return ID; }
	private:
		static std::atomic<uint32_t> hashId;
		uint32_t ID;
		
	};
	using ComponentHandle = std::shared_ptr<tComponent>;
}

