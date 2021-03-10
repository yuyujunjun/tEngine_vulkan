#pragma once
#include "Core.h"
#include <string>
#include <memory>
#include"tDescriptorPool.h"
namespace tEngine {
	namespace Reflector {
	
		 void reflectionShader(std::string jsonfile, std::vector<tDescSetsData>& descSetsData, GpuBlockBuffer& pushConstant,vk::ShaderStageFlags stageFlag);
		
	};


}

//extern std::map<std::string, pvr::GpuDatatypes> dataTypeMap;
//extern std::map<std::string, pvrvk::DescriptorType> descTypeMap;
