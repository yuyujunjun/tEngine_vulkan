#pragma once
#include"SimpleGeometry.h"
#include"stb_image/stb_image.h"
#include"vulkan/vulkan.hpp"
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
	struct ShaderAsset :public Asset {
		std::vector<uint32_t> shaderSource;
		std::string shaderReflection;
	};
	struct ImageAsset :public Asset {
		stbi_uc* pixels;
		bool enableRandomWrite = false;
		bool autoGenerateMips = false;
		bool useMipMap = false;
		vk::ImageType imageType = vk::ImageType::e2D;
		vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled;
		vk::Format format = vk::Format::eUndefined;
		//pvrvk::SampleCountFlags sampleCount = pvrvk::SampleCountFlags::e_1_BIT;
		vk::ImageTiling imageTiling = vk::ImageTiling::eOptimal;
		vk::MemoryPropertyFlags memoryProperty = vk::MemoryPropertyFlagBits::eDeviceLocal;
		int width, height, channels;
		int depth = 1;

		~ImageAsset() {
			stbi_image_free(pixels);
		}
	};
}