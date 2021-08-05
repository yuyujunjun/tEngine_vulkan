#pragma once
#include"Sampler.h"

#include<unordered_map>
#include"GraphicsState.h"
namespace tEngine {
	class tShaderInterface;
	class tShader;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;
	class tBuffer;
	using BufferHandle = std::shared_ptr<tBuffer>;
	class BufferRangeManager;
	struct GraphicsState;

	
	struct Material {
		Material(tShader* shader);
		
		struct value_offset {
			std::vector<uint8_t> data;
		//	size_t offset=-1;
			std::string bufferName="";
		};
		void SetValue(const std::string& name, const void* value, size_t size);
		template<typename T>
		void SetValue(const std::string& name, const T& value) {
			SetValue(name,&value,sizeof(T));
		}
		//Create buffer(if required) and update it, connect the buffer to its descriptorSet
		void flushBuffer();
		
		void SetBuffer(const std::string& name, const BufferHandle& buffer, size_t offset = 0);
		void SetImage(std::string name, ImageHandle image, vk::ImageView vkView = {}, StockSampler sampler = StockSampler::LinearClamp);
		
		tShaderInterface*  shader;
		GraphicsState graphicsState;
		~Material() {
			delete shader;
		}
	private:
		std::unordered_map<std::string, value_offset> storedValue;

	};
	struct View {
		std::shared_ptr<Material> material;

	};

}