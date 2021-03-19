#pragma once
#include"vulkan/vulkan.hpp"
namespace tEngine {
	class Device;
	 
	enum class StockSampler
	{
		NearestClamp,
		LinearClamp,
		TrilinearClamp,
		NearestWrap,
		LinearWrap,
		TrilinearWrap,
		NearestShadow,
		LinearShadow,
		LinearYUV420P,
		LinearYUV422P,
		LinearYUV444P,
		Count
	};
	class SamplerCreateInfo
	{
	public:
		VkFilter mag_filter;
		VkFilter min_filter;
		VkSamplerMipmapMode mipmap_mode;
		VkSamplerAddressMode address_mode_u;
		VkSamplerAddressMode address_mode_v;
		VkSamplerAddressMode address_mode_w;
		float mip_lod_bias;
		VkBool32 anisotropy_enable;
		float max_anisotropy;
		VkBool32 compare_enable;
		VkCompareOp compare_op;
		float min_lod;
		float max_lod;
		VkBorderColor border_color;
		VkBool32 unnormalized_coordinates;
	};
	class tSampler {
	public:
		

		tSampler(const Device* device, vk::Sampler sampler, const SamplerCreateInfo& info) :device(device), vksampler(sampler), info(info) {
		}
		~tSampler();
		const vk::Sampler& getVkHandle()const {
			return vksampler;
		}
		const SamplerCreateInfo& getCreateInfo()const {
			return info;
		}
	private:
		SamplerCreateInfo info;
		vk::Sampler vksampler;
		const Device* device;

	};
	using SamplerHandle = std::shared_ptr<tSampler>;
	inline  bool operator==(const SamplerHandle& a, const SamplerHandle& b) {
		return a->getVkHandle() == b->getVkHandle();
	}
	inline  bool operator != (const SamplerHandle& a, const SamplerHandle& b) {
		return !(a->getVkHandle() == b->getVkHandle());
	}
	VkSamplerCreateInfo fill_vk_sampler_info(const SamplerCreateInfo& sampler_info);
	SamplerHandle createSampler(const Device* device, const SamplerCreateInfo& sampler_info);

}