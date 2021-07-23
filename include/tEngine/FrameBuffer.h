#pragma once
#include"RingPool.h"
#include<assert.h>
#include<unordered_set>
#include<unordered_map>
#include<vulkan/vulkan.hpp>
namespace tEngine {
	class Device;
	
	class CommandBuffer;
	using CommandBufferHandle = std::shared_ptr <CommandBuffer>;
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;// ::SharedPtr;

	struct AttachmentInfo
	{
		uint32_t idx=0;
		vk::AttachmentDescription2 description;
		vk::ClearValue value;
	};



	struct tSubpass {
		friend class tRenderPass;

		tSubpass(uint32_t idx, tRenderPass& renderPass) :renderPass(renderPass), passIdx(idx) {}

		void addInput(std::string name, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);
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
		vk::SubpassDescription2& createDescription(VkPipelineBindPoint pipelineBindPoint);
		size_t getColorAttachmentCount()const {
			return (color.size() + resolved.size());
		}



	private:
		vk::SubpassDescription2 vkSubpassDescription;
		std::vector<vk::AttachmentReference2> input;
		std::vector<vk::AttachmentReference2> color;
		vk::AttachmentReference2 depth = {};
		std::vector<vk::AttachmentReference2> resolved;
		std::vector<uint32_t> preserve;

		uint32_t passIdx;
		tRenderPass& renderPass;

	};

	class tFrameBuffer;
	using FrameBufferHandle = std::shared_ptr<tFrameBuffer>;
	std::vector<vk::SubpassDependency2> SingleDependencies();
	std::vector<vk::SubpassDependency2> WriteBeforeShaderReadDependencies();
	
	class tRenderPass {
	public:
		
		const uint32_t allocatedFrameBufferCount = 8;
		using SharedPtr = std::shared_ptr<tRenderPass>;
		tRenderPass(const Device* device) :device(device) {

		}
		~tRenderPass();


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
		void SetAttachmentDescription(std::string name, vk::AttachmentDescription2& description) {
			getAttachment(name).description = description;
		}
		AttachmentInfo& getAttachment(std::string name,bool create=true) {
			auto iter = resource_to_idx.find(name);
			if (iter != resource_to_idx.end()) {
				return resource[iter->second];
			}
			else {
				assert(create&&"cannot find attachement with name");
				resource_to_idx[name] = static_cast<uint32_t>(resource.size());
				resource.emplace_back(AttachmentInfo());
				resource.back().idx = static_cast<uint32_t>(resource.size()) - 1;
				return resource.back();
			}
		}


		void SetDependencies(std::vector<vk::SubpassDependency2>& dependencies) {
			this->dependencies = dependencies;
		}
		void setupRenderPass(VkPipelineBindPoint bindp = VK_PIPELINE_BIND_POINT_GRAPHICS);
		void SetImageView(std::string name, const ImageHandle& handle, vk::ImageView view = {});
		//Create and set depthBuffer, if you already have depth buffer, use SetImageView
		void setTransientImageView(std::string name, vk::Extent2D extent = vk::Extent2D(-1, -1));
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
			getAttachment(name,false).value = value;
		}
		size_t getAttachmentCount() {
			return resource.size();
		}
		const std::vector<AttachmentInfo>& getAttachments() const {
			return resource;
		}
		
	private:

		vk::RenderPass vkRenderPass;
		const Device* device;
		std::vector<vk::SubpassDependency2> dependencies;
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
	using RenderPassHandle = std::shared_ptr<tRenderPass>;
	class tFrameBuffer {

	public:
		friend class tRenderPass;
		using SharedPtr = std::shared_ptr<tFrameBuffer>;
		tFrameBuffer(const Device* device) :device(device) {

		}
		~tFrameBuffer();

		vk::Rect2D getRenderArea() const {
			return vk::Rect2D({ 0, 0 }, { width, height });
		}
		const vk::Viewport getViewPort()const {
			return vk::Viewport(0.f, static_cast<float>(height), static_cast<float>(width), -static_cast<float>(height), 0.f, 1.f);
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
		const Device* device;
	};
	
}