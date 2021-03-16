#pragma once
#include"vulkan/vulkan.h"
#include"vulkan/vulkan.hpp"
#include"Core.h"
#include"tAsset.h"
#include"tEngineFunctional.h"
#include"tFormatInterfacce.h"

//Only support SharingMode: Exclusive, which means have to transfer resource when using the different queues
namespace tEngine {
	enum class BufferDomain
	{
		Device, // Device local. Probably not visible from CPU.
		LinkedDeviceHost, // On desktop, directly mapped VRAM over PCI.
		LinkedDeviceHostPreferDevice, // Prefer device local of host visible.
		Host, // Host-only, needs to be synced to GPU. Might be device local as well on iGPUs.
		CachedHost,
		CachedCoherentHostPreferCoherent, // Aim for both cached and coherent, but prefer COHERENT
		CachedCoherentHostPreferCached, // Aim for both cached and coherent, but prefer CACHED
	};
	enum BufferMiscFlagBits
	{
		BUFFER_MISC_ZERO_INITIALIZE_BIT = 1 << 0
	};
	using BufferMiscFlags = uint32_t;
	struct  BufferCreateInfo
	{
		BufferDomain domain = BufferDomain::Device;
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage=0;
		BufferMiscFlags misc = 0;
	};
	
	class tBuffer {
	public:
		
		tBuffer(Device* device,vk::Buffer buffer,const VmaAllocation& alloc, BufferCreateInfo& bufferInfo):device(device), bufferInfo(bufferInfo),alloc(alloc), vkHandle(buffer){
		}
		~tBuffer() {
			
			if (vkHandle) {
				device->freeAllocation(alloc);
	
				device->destroyBuffer(vkHandle);
				vkHandle = vk::Buffer();
			}
			
		}
		const vk::Buffer& getVkHandle()const {
			return vkHandle;
		}
		const VmaAllocation& getAllocation()const {
			return alloc;
		}
		const BufferCreateInfo& getCreateInfo()const {
			bufferInfo.size = alloc->GetSize();
			return bufferInfo;
			
		}
		const vk::DeviceSize getSize()const {
			return alloc->GetSize();
		}
		void Flush() {
			vmaFlushAllocation(device->getAllocator(), alloc, 0, bufferInfo.size);
		}
	private:
		vk::Buffer vkHandle;
		weakDevice device;
		VmaAllocation alloc;
		BufferCreateInfo& bufferInfo;
	};

	struct InitialImageBuffer
	{
		BufferHandle buffer;
		std::vector<vk::BufferImageCopy> blits;
	};
	
	inline  bool operator==(const BufferHandle& a,const BufferHandle& b) {
		return a->getVkHandle() == b->getVkHandle();
	}
	inline  bool operator != (const BufferHandle& a, const BufferHandle& b) {
		return !(a->getVkHandle() == b->getVkHandle());
	}
	struct SamplerCreateInfo
	{
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
		DECLARE_SHARED(tSampler);

		tSampler(Device* device,vk::Sampler sampler,const SamplerCreateInfo& info):device(device),vksampler(sampler),info(info) {
		}
		~tSampler() {
			if (vksampler) {
				device->destroySampler(vksampler); 
				vksampler = vk::Sampler();
			}
		}
		const vk::Sampler& getVkHandle()const {
			return vksampler;
		}
		const SamplerCreateInfo& getCreateInfo()const {
			return info;
		}
	private:
		SamplerCreateInfo info;
		vk::Sampler vksampler;
		Device* device;
		
	};
	
	inline  bool operator==(const SamplerHandle& a, const SamplerHandle& b) {
		return a->getVkHandle() == b->getVkHandle();
	}
	inline  bool operator != (const SamplerHandle& a, const SamplerHandle& b) {
		return !(a->getVkHandle() == b->getVkHandle());
	}
	
	enum ImageViewMiscFlagBits
	{
		IMAGE_VIEW_MISC_FORCE_ARRAY_BIT = 1 << 0
	};
	using ImageViewMiscFlags = uint32_t;
	class tImage;
	struct ImageViewCreateInfo
	{
		tImage* image = nullptr;
		VkFormat format = VK_FORMAT_UNDEFINED;
		unsigned base_level = 0;
		unsigned levels = VK_REMAINING_MIP_LEVELS;
		unsigned base_layer = 0;
		unsigned layers = VK_REMAINING_ARRAY_LAYERS;
		VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		ImageViewMiscFlags misc = 0;
		VkComponentMapping swizzle = {
				VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
		};
	};
	//Must depend on the specifical image, so createde from the image
	//One image can have different views, tImageView include all views the image have
	class tImageView {
	public:
		
		
		tImageView(Device* device,vk::ImageView default_view, ImageViewCreateInfo& info) :device(device),view(default_view), info(info) {
		}
		~tImageView() {
			if (view) {
				
					device->destroyImageView(view);
					view = vk::ImageView();
				
			}
			if (depth_view) {
				device->destroyImageView(depth_view);
			}
			if (stencil_view) {
				device->destroyImageView(stencil_view);
			}
			if (unorm_view) {
				device->destroyImageView(unorm_view);
			}
			if (srgb_view) {
				device->destroyImageView(srgb_view);
			}
			for (auto& view : render_target_views) {
				device->destroyImageView(view);
			}
		}
		VkFormat getFormat() const {
			return  info.format;
		}
		void set_alt_views(VkImageView depth, VkImageView stencil)
		{
			VK_ASSERT(depth_view == VK_NULL_HANDLE);
			VK_ASSERT(stencil_view == VK_NULL_HANDLE);
			depth_view = depth;
			stencil_view = stencil;
		}
		void set_render_target_views(std::vector<VkImageView> views)
		{
			VK_ASSERT(render_target_views.empty());
			render_target_views = std::move(views);
		}
		void set_unorm_view(VkImageView view_)
		{
			VK_ASSERT(unorm_view == VK_NULL_HANDLE);
			unorm_view = view_;
		}

		void set_srgb_view(VkImageView view_)
		{
			VK_ASSERT(srgb_view == VK_NULL_HANDLE);
			srgb_view = view_;
		}



		VkImageView get_render_target_view(unsigned layer) const;

		// Gets an image view which only includes floating point domains.
		// Takes effect when we want to sample from an image which is Depth/Stencil,
		// but we only want to sample depth.
		VkImageView get_float_view() const
		{
			return depth_view != VK_NULL_HANDLE ? depth_view : view;
		}

		// Gets an image view which only includes integer domains.
		// Takes effect when we want to sample from an image which is Depth/Stencil,
		// but we only want to sample stencil.
		VkImageView get_integer_view() const
		{
			return stencil_view != VK_NULL_HANDLE ? stencil_view : view;
		}

		VkImageView get_unorm_view() const
		{
			return unorm_view;
		}

		VkImageView get_srgb_view() const
		{
			return srgb_view;
		}

		VkFormat get_format() const
		{
			return info.format;
		}

		const tImage& get_image() const
		{
			return *info.image;
		}

		tImage* get_image()
		{
			return info.image;
		}

		const ImageViewCreateInfo& get_create_info() const
		{
			return info;
		}
		// By default, gets a combined view which includes all aspects in the image.
// This would be used mostly for render targets.
		const vk::ImageView& getDefaultView()const {
			return view;
		}
		bool operator==(const tImageView& v)const {
			return v.view == view &&
				v.render_target_views == render_target_views && depth_view == v.depth_view && stencil_view == v.stencil_view
				&& srgb_view == v.srgb_view;

		}
		bool operator!=(const tImageView& v)const {
			return !(*this == v);
		}
	private:
		vk::ImageView view;
		std::vector<VkImageView> render_target_views;
		VkImageView depth_view = VK_NULL_HANDLE;
		VkImageView stencil_view = VK_NULL_HANDLE;
		VkImageView unorm_view = VK_NULL_HANDLE;
		VkImageView srgb_view = VK_NULL_HANDLE;
		ImageViewCreateInfo info;
		Device* device;
	};
	using ImageviewHandle = std::shared_ptr<tImageView>;
	bool operator==(const ImageviewHandle& a, const ImageviewHandle& b) {
		assert(false);
	}
	bool operator != (const ImageviewHandle& a, const ImageviewHandle& b) {
		assert(false);
	}
	//ImageviewHandle CreateImageViewWithImage(uniqueDevice& device, vk::ImageCreateInfo imageInfo, vk::ImageViewCreateInfo viewInfo, vk::MemoryPropertyFlags& memoryProperties);
	//ImageviewHandle CreateImageViewWithImage(uniqueDevice& device, std::shared_ptr<ImageAsset>& asset, const std::shared_ptr<CommandBuffer>& cb);
//	void CopyBufferToImage(const std::shared_ptr<CommandBuffer>& cb, tBuffer::SharedPtr buffer, ImageviewHandle imageView, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout);






	enum class ImageDomain
	{
		Physical,
		Transient,
		LinearHostCached,
		LinearHost
	};
	enum ImageMiscFlagBits
	{
		IMAGE_MISC_GENERATE_MIPS_BIT = 1 << 0,
		IMAGE_MISC_FORCE_ARRAY_BIT = 1 << 1,
		IMAGE_MISC_MUTABLE_SRGB_BIT = 1 << 2,
		IMAGE_MISC_CONCURRENT_QUEUE_GRAPHICS_BIT = 1 << 3,
		IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_COMPUTE_BIT = 1 << 4,
		IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_GRAPHICS_BIT = 1 << 5,
		IMAGE_MISC_CONCURRENT_QUEUE_ASYNC_TRANSFER_BIT = 1 << 6,
		IMAGE_MISC_VERIFY_FORMAT_FEATURE_SAMPLED_LINEAR_FILTER_BIT = 1 << 7,
		IMAGE_MISC_LINEAR_IMAGE_IGNORE_DEVICE_LOCAL_BIT = 1 << 8,
		IMAGE_MISC_FORCE_NO_DEDICATED_BIT = 1 << 9
	};
	using ImageMiscFlags = uint32_t;
	struct ImageCreateInfo
	{
		ImageDomain domain = ImageDomain::Physical;
		unsigned width = 0;
		unsigned height = 0;
		unsigned depth = 1;
		unsigned levels = 1;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageType type = VK_IMAGE_TYPE_2D;
		unsigned layers = 1;
		VkImageUsageFlags usage = 0;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkImageCreateFlags flags = 0;
		ImageMiscFlags misc = 0;
		VkImageLayout initial_layout = VK_IMAGE_LAYOUT_GENERAL;
		VkComponentMapping swizzle = {
				VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
		};
		const VmaAllocation** memory_aliases = nullptr;
		unsigned num_memory_aliases = 0;

		static ImageCreateInfo immutable_image(const TextureFormatLayout& layout)
		{
		
			ImageCreateInfo info;
			info.width = layout.get_width();
			info.height = layout.get_height();
			info.type = layout.get_image_type();
			info.depth = layout.get_depth();
			info.format = layout.get_format();
			info.layers = layout.get_layers();
			info.levels = layout.get_levels();
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			info.initial_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.domain = ImageDomain::Physical;
			return info;
		}

		static ImageCreateInfo immutable_2d_image(unsigned width, unsigned height, VkFormat format, bool mipmapped = false)
		{
			ImageCreateInfo info;
			info.width = width;
			info.height = height;
			info.depth = 1;
			info.levels = mipmapped ? 0u : 1u;
			info.format = format;
			info.type = VK_IMAGE_TYPE_2D;
			info.layers = 1;
			info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.flags = 0;
			info.misc = mipmapped ? unsigned(IMAGE_MISC_GENERATE_MIPS_BIT) : 0u;
			info.initial_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			return info;
		}

		static ImageCreateInfo
			immutable_3d_image(unsigned width, unsigned height, unsigned depth, VkFormat format, bool mipmapped = false)
		{
			ImageCreateInfo info = immutable_2d_image(width, height, format, mipmapped);
			info.depth = depth;
			info.type = VK_IMAGE_TYPE_3D;
			return info;
		}

		static ImageCreateInfo render_target(unsigned width, unsigned height, VkFormat format)
		{
			ImageCreateInfo info;
			info.width = width;
			info.height = height;
			info.depth = 1;
			info.levels = 1;
			info.format = format;
			info.type = VK_IMAGE_TYPE_2D;
			info.layers = 1;
			info.usage = (format_has_depth_or_stencil_aspect(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT :
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) |
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.flags = 0;
			info.misc = 0;
			info.initial_layout = format_has_depth_or_stencil_aspect(format) ?
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			return info;
		}

		static ImageCreateInfo transient_render_target(unsigned width, unsigned height, VkFormat format)
		{
			ImageCreateInfo info;
			info.domain = ImageDomain::Transient;
			info.width = width;
			info.height = height;
			info.depth = 1;
			info.levels = 1;
			info.format = format;
			info.type = VK_IMAGE_TYPE_2D;
			info.layers = 1;
			info.usage = (format_has_depth_or_stencil_aspect(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT :
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) |
				VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			info.samples = VK_SAMPLE_COUNT_1_BIT;
			info.flags = 0;
			info.misc = 0;
			info.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			return info;
		}

		static uint32_t compute_view_formats(const ImageCreateInfo& info, VkFormat* formats)
		{
			if ((info.misc & IMAGE_MISC_MUTABLE_SRGB_BIT) == 0)
				return 0;

			switch (info.format)
			{
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SRGB:
				formats[0] = VK_FORMAT_R8G8B8A8_UNORM;
				formats[1] = VK_FORMAT_R8G8B8A8_SRGB;
				return 2;

			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SRGB:
				formats[0] = VK_FORMAT_B8G8R8A8_UNORM;
				formats[1] = VK_FORMAT_B8G8R8A8_SRGB;
				return 2;

			case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
				formats[0] = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
				formats[1] = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
				return 2;

			default:
				return 0;
			}
		}
	};

	struct tImage {
	public:
		DECLARE_NO_COPY_SEMANTICS(tImage)
		friend class tImageView;
		
		
		tImage(Device* device, vk::Image image, vk::ImageView default_view, const VmaAllocation& alloc,
			const ImageCreateInfo& cinfo,VkImageViewType view_type) :vkImage(image), device(device), create_info(cinfo), alloc(alloc),view(view){
			ImageViewCreateInfo info;
			info.image = this;
			info.view_type = view_type;
			info.format = create_info.format;
			info.base_level = 0;
			info.levels = create_info.levels;
			info.base_layer = 0;
			info.layers = create_info.layers;
			view = std::make_shared<tImageView>(device, default_view,info);
			
		}
		
		const tImageView* get_view() const
		{
			VK_ASSERT(view);
			return view.get();
		}

		ImageviewHandle get_view()
		{
			VK_ASSERT(view);
			return view;
		}

		VkImage get_image() const
		{
			return vkImage;
		}

		VkFormat get_format() const
		{
			return create_info.format;
		}

		uint32_t get_width(uint32_t lod = 0) const
		{
			return std::max(1u, create_info.width >> lod);
		}

		uint32_t get_height(uint32_t lod = 0) const
		{
			return std::max(1u, create_info.height >> lod);
		}

		uint32_t get_depth(uint32_t lod = 0) const
		{
			return std::max(1u, create_info.depth >> lod);
		}

		const ImageCreateInfo& get_create_info() const
		{
			return create_info;
		}

	
		bool is_swapchain_image() const
		{
			return swapchain_layout != VK_IMAGE_LAYOUT_UNDEFINED;
		}

		VkImageLayout get_swapchain_layout() const
		{
			return swapchain_layout;
		}

		void set_swapchain_layout(VkImageLayout layout)
		{
			swapchain_layout = layout;
		}

		void set_stage_flags(VkPipelineStageFlags flags)
		{
			stage_flags = flags;
		}

		void set_access_flags(VkAccessFlags flags)
		{
			access_flags = flags;
		}

		VkPipelineStageFlags get_stage_flags() const
		{
			return stage_flags;
		}

		VkAccessFlags get_access_flags() const
		{
			return access_flags;
		}

		const VmaAllocation& get_allocation() const
		{
			return alloc;
		}

		~tImage() {
			
			if (vkImage) {
					device->destroyImage(vkImage);
					if (alloc != nullptr) {
						device->getAllocator()->FreeMemory(1, &alloc);
					}
				
					vkImage = vk::Image();
				
			}

		}
		

		
		void _bakeImageAsset(std::shared_ptr<ImageAsset>& asset) {
			this->asset = asset;
		}
		
		const vk::Image& getVkHandle()const {
			return vkImage;
		}
	private:
		vk::Image vkImage;
		ImageviewHandle view;
		VmaAllocation alloc;
		ImageCreateInfo create_info;
		VkPipelineStageFlags stage_flags = 0;
		VkAccessFlags access_flags = 0;
		VkImageLayout swapchain_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		bool owns_image = true;
		bool owns_memory_allocation = true;
		Device* device;
	
		std::shared_ptr<ImageAsset> asset;
		
	};
	
	inline  bool operator==(const ImageHandle& a, const ImageHandle& b) {
		return a->getVkHandle() == b->getVkHandle();
	}
	inline  bool operator != (const ImageHandle& a, const ImageHandle& b) {
		return !(a->getVkHandle() == b->getVkHandle());
	}
	class tSwapChain {
	public:
		using SharedPtr = std::shared_ptr<tSwapChain>;
		tSwapChain(	Device* device,vk::SwapchainKHR const& surface);
		void setImages(std::vector<ImageHandle>& images) {
			imageList = images;
		}
		size_t getSwapchainLength() {
			return imageList.size();
		}
		vk::Format getFormat() {
			imageList[0]->get_format();
		}
		~tSwapChain() {
			if (swapChain) {
				device->destroySwapchainKHR(swapChain);
			}
		}
	private:
		weakDevice device;
		vk::SwapchainKHR swapChain;
		std::vector<ImageHandle> imageList;
	};

}