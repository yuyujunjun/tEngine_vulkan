#pragma once
#include <string>
#include <memory>
#include"vulkan/vulkan.hpp"
#include"GpuBlock.h"
namespace tEngine {
	struct tDescSetsDataWithSetNumber;
	namespace Reflector {
	
		 void reflectionShader(std::string jsonfile, std::vector<tDescSetsDataWithSetNumber>& descSetsData, GpuBlockBuffer& pushConstant,vk::ShaderStageFlags stageFlag);
		
	};


}

//extern std::map<std::string, pvr::GpuDatatypes> dataTypeMap;
//extern std::map<std::string, pvrvk::DescriptorType> descTypeMap;
