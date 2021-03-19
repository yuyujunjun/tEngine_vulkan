#pragma once
#include"tAsset.h"
#include"Tgine.h"
//Only support SharingMode: Exclusive, which means have to transfer resource when using the different queues
namespace tEngine {

	enum ImageViewMiscFlagBits
	{
		IMAGE_VIEW_MISC_FORCE_ARRAY_BIT = 1 << 0
	};
	using ImageViewMiscFlags = uint32_t;
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
	class  BufferCreateInfo
	{
	public:
		BufferDomain domain = BufferDomain::Device;
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
		BufferMiscFlags misc = 0;
	};
	class tBuffer {
	public:
		
		tBuffer(const Device* device,vk::Buffer buffer,const VmaAllocation& alloc,const BufferCreateInfo& bufferInfo):device(device), bufferInfo(bufferInfo),alloc(alloc), vkHandle(buffer){
		
		}
		~tBuffer();
		const vk::Buffer& getVkHandle()const {
			return vkHandle;
		}
		const VmaAllocation& getAllocation()const {
			return alloc;
		}
		const BufferCreateInfo& getCreateInfo()const {
		//	bufferInfo.size = alloc->GetSize();
			return bufferInfo;
			
		}
		void setRange(const void* data, size_t offset, size_t size);
		const vk::DeviceSize getSize()const;
		void Flush();
	private:
		vk::Buffer vkHandle;
		weakDevice device;
		VmaAllocation alloc;
		BufferCreateInfo bufferInfo;
		vk::MemoryPropertyFlags memoryProperty;
	
	};
	//offset update must align the minAlignmentOffset(each update must be a buffer update)
	class BufferRangeManager {
	public:
		BufferRangeManager(const BufferHandle buffer, size_t rangeSize = -1, size_t initialOffset = 0) :handle(buffer), rangeSize(rangeSize), initialOff(initialOffset) {
			if (rangeSize == -1)rangeSize = buffer->getSize();
		}
		size_t SetRangeIncremental(void* data);
		//return current offset
		void SetRangeNoLock(void* data);
		const  BufferHandle& buffer() const {
			return handle;
		}
		void NextRangenoLock() {
			offset = offset + rangeSize * 2 <= handle->getSize() ? offset + rangeSize : initialOff;
		}
		size_t getOffset()const {
			return offset;
		}
	private:

		size_t rangeSize = 0;
		//std::mutex mtx;
		size_t initialOff = 0;
		size_t offset = 0;
		const BufferHandle handle;
	};
	class InitialImageBuffer
	{
	public:
		BufferHandle buffer;
		std::vector<vk::BufferImageCopy> blits;
	};
	
	inline  bool operator==(const BufferHandle& a,const BufferHandle& b) {
		return a->getVkHandle() == b->getVkHandle();
	}
	inline  bool operator != (const BufferHandle& a, const BufferHandle& b) {
		return !(a->getVkHandle() == b->getVkHandle());
	}

	
	class tSampler {
	public:
		DECLARE_SHARED(tSampler);

		tSampler(const Device* device,vk::Sampler sampler,const SamplerCreateInfo& info):device(device),vksampler(sampler),info(info) {
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
		weakDevice device;
		
	};
	
	inline  bool operator==(const SamplerHandle& a, const SamplerHandle& b) {
		return a->getVkHandle() == b->getVkHandle();
	}
	inline  bool operator != (const SamplerHandle& a, const SamplerHandle& b) {
		return !(a->getVkHandle() == b->getVkHandle());
	}
	

	class tImage;
	
	//Must depend on the specifical image, so createde from the image
	//One image can have different views, tImageView include all views the image have
	class tImageView {
	public:
		
		
		tImageView(const Device* device,vk::ImageView default_view, ImageViewCreateInfo& info) :device(device),view(default_view), info(info) {
		}
		~tImageView();
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
		weakDevice device;
	};
	using ImageviewHandle = std::shared_ptr<tImageView>;
	inline bool operator==(const ImageviewHandle& a, const ImageviewHandle& b) {
		assert(false);
		return true;
	}
	inline bool operator != (const ImageviewHandle& a, const ImageviewHandle& b) {
		assert(false);
		return !(b==a);
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
	class ImageCreateInfo
	{
	public:
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

		static ImageCreateInfo immutable_image(const TextureFormatLayout& layout);
		static ImageCreateInfo immutable_2d_image(unsigned width, unsigned height, VkFormat format, bool mipmapped = false);
		static ImageCreateInfo immutable_3d_image(unsigned width, unsigned height, unsigned depth, VkFormat format, bool mipmapped = false);
		static ImageCreateInfo render_target(unsigned width, unsigned height, VkFormat format);
		static ImageCreateInfo transient_render_target(unsigned width, unsigned height, VkFormat format);
		static uint32_t compute_view_formats(const ImageCreateInfo& info, VkFormat* formats);
	};

	class tImage{
	public:
		DECLARE_NO_COPY_SEMANTICS(tImage)
		friend class tImageView;
		
		
		tImage(const Device* device, vk::Image image, vk::ImageView default_view, const VmaAllocation& alloc,
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
		void ownsImage(bool own) {
			owns_image = own;
		}
		~tImage();
		

		
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
		weakDevice device;
	
		std::shared_ptr<ImageAsset> asset;
		
	};
	
	inline  bool operator==(const ImageHandle& a, const ImageHandle& b) {
		return a->getVkHandle() == b->getVkHandle();
	}
	inline  bool operator != (const ImageHandle& a, const ImageHandle& b) {
		return !(a->getVkHandle() == b->getVkHandle());
	}
	


	SwapChainHandle createSwapChain(Device* device, vk::SurfaceKHR const& surface, vk::Extent2D const& extent, vk::ImageUsageFlags usage, vk::SwapchainKHR const& oldSwapChain, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex);
	void updateBufferUsingStageBuffer(const Device* device, BufferHandle& buffer, CommandBufferHandle& cb, const void* data, size_t size, size_t offset = 0);
	void fillBufferUsingStageBuffer(const Device* device, BufferHandle& handle, CommandBufferHandle& cb, uint32_t zero, size_t size, size_t offset = 0);
	void find_memory_type(ImageDomain domain, VmaAllocationCreateInfo& info);
	void find_memory_type(BufferDomain domain, VmaAllocationCreateInfo& info);
	VkSamplerCreateInfo fill_vk_sampler_info(const SamplerCreateInfo& sampler_info);
}