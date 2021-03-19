#pragma once
#include <string>
#include <memory>
#include"tDescriptorShared.h"
#include"tGpuBlock.h"
namespace tEngine {
	namespace Reflector {
	
		 void reflectionShader(std::string jsonfile, std::vector<tDescSetsDataWithSetNumber>& descSetsData, GpuBlockBuffer& pushConstant,vk::ShaderStageFlags stageFlag);
		
	};


}

//extern std::map<std::string, pvr::GpuDatatypes> dataTypeMap;
//extern std::map<std::string, pvrvk::DescriptorType> descTypeMap;
