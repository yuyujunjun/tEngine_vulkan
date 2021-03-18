#pragma once
#include"tDevice.h"
#include"tResource.h"
#include<unordered_set>
#include<unordered_map>
namespace tEngine {



	enum SizeClass
	{
		Absolute,
		SwapchainRelative,
		InputRelative
	};

	struct AttachmentInfo
	{
		uint32_t idx;
		vk::AttachmentDescription description;
		vk::ClearValue value;
	};

	struct BufferInfo
	{
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
		bool persistent = true;

		bool operator==(const BufferInfo& other) const
		{
			return size == other.size &&
				usage == other.usage &&
				persistent == other.persistent;
		}

		bool operator!=(const BufferInfo& other) const
		{
			return !(*this == other);
		}
	};



	struct tSubpass {
		friend class tRenderPass;

		tSubpass(uint32_t idx, tRenderPass& renderPass) :renderPass(renderPass), passIdx(idx) {}


		void addColorOutput(std::string name, vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal);
		void setDepth(std::string name, vk::ImageLayout layout = vk::ImageLayout::eDepthStencilAttachmentOptimal);
		void addResolvedOutput(std::string name, vk::ImageLayout layout);
		void addPreserve(std::string name);
		bool operator==(const tSubpass& subpass) const {

			return (vkSubpassDescription == subpass.vkSubpassDescription)
				&& (input == subpass.input)
				&& (color == subpass.color)
				&& (depth == subpass.depth)
				&& (resolved == subpass.resolved);
		}
		bool operator!=(const tSubpass& subpass)const {
			return !(operator==(subpass));
		}
		vk::SubpassDescription& createDescription(VkPipelineBindPoint pipelineBindPoint);
		size_t getColorAttachmentCount()const {
			return (color.size() + resolved.size());
		}



	private:
		vk::SubpassDescription vkSubpassDescription;
		std::vector<vk::AttachmentReference> input;
		std::vector<vk::AttachmentReference> color;
		vk::AttachmentReference depth = {};
		std::vector<vk::AttachmentReference> resolved;
		std::vector<uint32_t> preserve;

		uint32_t passIdx;
		tRenderPass& renderPass;

	};

	struct tFrameBuffer;
	std::vector<vk::SubpassDependency> SingleDependencies();
	std::vector<vk::SubpassDependency> WriteBeforeShaderReadDependencies();
	class tRenderPass {
	public:
		const uint32_t allocatedFrameBufferCount = 8;
		using SharedPtr = std::shared_ptr<tRenderPass>;
		tRenderPass(Device* device) :device(device) {

		}
		~tRenderPass() {
			if (vkRenderPass) {
				device->destroyRenderPass(vkRenderPass);
			}
		}


		const vk::RenderPass getVkHandle()const {
			return vkRenderPass;
		}

		tSubpass& getPass(std::string name) {
			auto iter = pass_to_idx.find(name);
			if (iter != pass_to_idx.end()) {
				return *passes[iter->second].get();
			}
			else {
				auto idx = passes.size();
				passes.emplace_back(std::make_unique<tSubpass>(idx, *this));
				pass_to_idx[name] = static_cast<uint32_t>(idx);
				return *passes.back().get();
			}

		}
		void SetAttachmentDescription(std::string name, vk::AttachmentDescription& description) {
			getAttachment(name).description = description;
		}
		AttachmentInfo& getAttachment(std::string name) {
			auto iter = resource_to_idx.find(name);
			if (iter != resource_to_idx.end()) {
				return resource[iter->second];
			}
			else {
				resource_to_idx[name] = static_cast<uint32_t>(resource.size());

				resource.emplace_back(AttachmentInfo());
				resource.back().idx = static_cast<uint32_t>(resource.size()) - 1;
				return resource.back();
			}
		}


		void SetDependencies(std::vector<vk::SubpassDependency>& dependencies) {
			this->dependencies = dependencies;
		}
		void setupRenderPass(VkPipelineBindPoint bindp = VK_PIPELINE_BIND_POINT_GRAPHICS);
		void SetImageView(std::string name, const ImageHandle& handle, vk::ImageView view = {}) {

			assert(resource_to_idx.count(name) != 0);

			images[resource_to_idx[name]] = handle;
			if (view)
				views[resource_to_idx[name]] = view;
			else views[resource_to_idx[name]] = handle->get_view()->getDefaultView();

		}
		void SetRenderFunctor(uint32_t subpass, std::function<void(CommandBufferHandle&, tRenderPass*, uint32_t subpass)> fun) {
			//if (renderFunction.size() <= subpass) { renderFunction.resize(subpass + 1); }
			renderFunction[subpass] = std::move(fun);
		}

		void Render(CommandBufferHandle& cb) {
			for (uint32_t pass = 0; pass < passes.size(); ++pass) {
				renderFunction[pass](cb, this, pass);
			}

		}
		vk::RenderPass getVkHandle() {
			return vkRenderPass;
		}
		FrameBufferHandle requestFrameBuffer();
		const uint32_t getSubpassId(std::string name)const {
			return pass_to_idx.at(name);
		}

		const tSubpass* getSubpass(uint32_t idx)const {
			return passes[idx].get();
		}
		const tSubpass* getSubpass(std::string name)const {
			return getSubpass(getSubpassId(name));
		}
		const std::vector<vk::ImageView>& getViews()const {
			return views;
		}
		const std::vector<ImageHandle>& getImages()const {
			return images;
		}
		void setClearValue(std::string name, std::array<float, 4> color) {
			assert(resource_to_idx.count(name) != 0);
			setClearValue(name, vk::ClearValue(vk::ClearColorValue(color)));
		}
		void setDepthStencilValue(std::string name, float depth, uint32_t stencil = 0) {
			setClearValue(name, vk::ClearValue(vk::ClearDepthStencilValue(depth, stencil)));
		}
		void setClearValue(std::string name, vk::ClearValue value) {
			getAttachment(name).value = value;
		}
		size_t getAttachmentCount() {
			return resource.size();
		}
		const std::vector<AttachmentInfo>& getAttachments() const {
			return resource;
		}
	private:

		vk::RenderPass vkRenderPass;
		weakDevice device;
		std::vector<vk::SubpassDependency> dependencies;
		std::vector<std::unique_ptr<tSubpass>> passes;
		std::vector<AttachmentInfo> resource;
		//std::vector<std::unique_ptr<RenderResource>> resources;
		std::unordered_map<std::string, uint32_t>pass_to_idx;
		std::unordered_map<std::string, uint32_t> resource_to_idx;
		std::vector<ImageHandle> images;
		std::vector<vk::ImageView> views;

		//allocate
		RingPool<tFrameBuffer, std::vector<vk::ImageView>> frameBuffersPool;

		//Function
		std::vector<std::function<void(CommandBufferHandle&, tRenderPass*, uint32_t subpass)>> renderFunction;
		//bool imageViewDirty = false;
	};
	struct tFrameBuffer {

	public:
		friend class tRenderPass;
		using SharedPtr = std::shared_ptr<tFrameBuffer>;
		tFrameBuffer(weakDevice device) :device(device) {

		}
		~tFrameBuffer() {
			if (vkFrameBuffer) {

				device->destroyFramebuffer(vkFrameBuffer);

			}
		}

		vk::Rect2D getRenderArea() const {
			return vk::Rect2D({ 0, 0 }, { width, height });
		}
		vk::Viewport getViewPort()const {
			return vk::Viewport(0, 0, width, height, 0, 1);
		}
		void setupFrameBuffer();
		const vk::Framebuffer getVkHandle() const {
			return vkFrameBuffer;
		}
	private:
		vk::Framebuffer vkFrameBuffer;
		uint32_t width = (uint32_t)-1;
		uint32_t height = (uint32_t)-1;
		tRenderPass* pass=nullptr;
		weakDevice device;
	};
	RenderPassHandle getSingleRenderpass(Device* device);
}