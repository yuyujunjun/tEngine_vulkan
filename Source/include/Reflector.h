#pragma once
#include "Tgine.h"
#include <string>
#include <memory>
#include"tDescriptorPool.h"
namespace tEngine {
	namespace Reflector {
	
		 void reflectionShader(std::string jsonfile, std::vector<tDescSetsDataWithSetNumber>& descSetsData, GpuBlockBuffer& pushConstant,vk::ShaderStageFlags stageFlag);
		
	};


}

//extern std::map<std::string, pvr::GpuDatatypes> dataTypeMap;
//extern std::map<std::string, pvrvk::DescriptorType> descTypeMap;
