
#include "cereal/external//rapidjson/document.h"
#include "Reflector.h"
#include <string>
#include <vector>
#include <stdint.h>
#include <cereal/external/rapidjson/reader.h>
#include <cereal/external/rapidjson/document.h>
#include<string>
#include<fstream>
#include<iostream>

#include<map>
#include"GpuBlock.h"
#include"DescriptorShared.h"
//#include "tPSystem.h"
//#include "Glow.h"


namespace tEngine {
	std::map<std::string, vk::DescriptorType> descTypeMap = {
		{"UNIFORM", vk::DescriptorType::eUniformBufferDynamic },
		{"BUFFER",vk::DescriptorType::eStorageBufferDynamic },
		{"IMAGE_SAMPLER", vk::DescriptorType::eCombinedImageSampler },
		{"SEPARATE_IMAGE",vk::DescriptorType::eSampledImage},
		{"SEPARATE_SAMPLER",vk::DescriptorType::eSampler}
	};


	void Reflector::reflectionShader(std::string jsonfile,std::vector<tDescSetsDataWithSetNumber>& descSetsData,GpuBlockBuffer& pushConstant, vk::ShaderStageFlags stageFlag) {
	//	std::unordered_map<std::string, GpuBlockBuffer> blocks;
		rapidjson::Document document;
		//std::unordered_map<uint32_t, DescriptorLayoutCreateInfo> sets;

		document.Parse(jsonfile.data(), jsonfile.size());
		const rapidjson::Value& descriptors = document["descriptor_sets"];
		descSetsData.reserve(descriptors.Size()+descSetsData.size());
		for (rapidjson::SizeType i = 0; i < descriptors.Size(); ++i) {
			descSetsData.push_back(tDescSetsDataWithSetNumber());
			uint32_t set_number = descriptors[i].GetObject()["set"].GetInt();
			auto& m_set = descSetsData.back();
			m_set.set_number = set_number;
			//Set binding
			vk::DescriptorSetLayoutBinding m_binding;
			m_binding.descriptorType = descTypeMap[descriptors[i].GetObject()["type"].GetString()];
			m_binding.binding = descriptors[i].GetObject()["binding"].GetInt();
			m_binding.descriptorCount = 1;
			m_binding.stageFlags = stageFlag;
			m_set.data.bindings.push_back(m_binding);
			//Set block
			GpuBlockBuffer block;
			block.name = descriptors[i].GetObject()["name"].GetString();
			const rapidjson::Value& members = descriptors[i].GetObject()["members"];
			int offset = 0;
			//遍历Member
			for (rapidjson::SizeType memberId = 0; memberId < members.Size(); ++memberId) {
				GpuBlockMember data;
				data.name = members[memberId].GetObject()["name"].GetString();
				data.offset = members[memberId].GetObject()["offset"].GetInt();
				data.size = members[memberId].GetObject()["size"].GetInt();
				offset += data.size;
				block.push_back(data);
			}
			
			m_set.blockBuffers[m_binding.binding] = block;
		}
		MergeSet(descSetsData);
		if (document.HasMember("push_constants")) {
			const rapidjson::Value& constants = document["push_constants"].GetObject()["elements"];
			int offset = 0;
			for (rapidjson::SizeType i = 0; i < constants.Size(); ++i) {
				GpuBlockMember data;
				data.name = constants[i].GetObject()["name"].GetString();
				data.offset = constants[i].GetObject()["offset"].GetInt();
				data.size = constants[i].GetObject()["size"].GetInt();
				assert(offset == data.offset);
				offset += data.size;
				pushConstant.push_back(data);
			}
		}

		
	}
	

	



}