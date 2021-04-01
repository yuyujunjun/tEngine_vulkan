#pragma once
#include"Sampler.h"

#include<unordered_map>
namespace tEngine {
	class tShaderInterface;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class BufferRangeManager;
	struct Material {
		Material(std::shared_ptr<tShaderInterface> shader);
		struct value_offset {
			std::vector<uint8_t> data;
			size_t offset=-1;
			std::string bufferName="";
		};
		void SetValue(const std::string& name, const void* value, size_t size);
		template<typename T>
		void SetValue(const std::string& name, const T& value) {
			SetValue(name,&value,sizeof(T));
		}
		void flushMaterialState();
		
		void SetBuffer(const std::string& name, const BufferHandle& buffer, size_t offset = 0);
		void SetImage(std::string name, ImageHandle image, vk::ImageView vkView = {}, StockSampler sampler = StockSampler::LinearClamp);
		std::shared_ptr<tShaderInterface> shader;
	private:
		std::unordered_map<std::string, value_offset> storedValue;

	};
		
	
}