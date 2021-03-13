#pragma once
#include"Core.h"
#include"tResource.h"
namespace tEngine {

	inline vk::AttachmentDescription createDepthStencilDescription(vk::Format format, vk::ImageLayout initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
		vk::ImageLayout finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare, vk::SampleCountFlagBits numSamples = vk::SampleCountFlagBits::e1)
	{
		vk::AttachmentDescription attach = { {}, format, numSamples, loadOp, storeOp, stencilLoadOp, stencilStoreOp, initialLayout, finalLayout };
		return attach;
	}
	inline vk::AttachmentDescription createColorDescription(vk::Format format, vk::ImageLayout initialLayout = vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore, vk::SampleCountFlagBits numSamples = vk::SampleCountFlagBits::e1)
	{
		vk::AttachmentDescription attach = { {}, format, numSamples, loadOp, storeOp, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, initialLayout, finalLayout };
		return attach;
	}


	//Advice by Nvidia tips
	//Use VK_IMAGE_LAYOUT_UNDEFINED when the previous content of the image is not needed.
	inline void UpdateEfficiency(vk::AttachmentDescription& description) {
		auto loadOp = description.loadOp;
		if (description.initialLayout == vk::ImageLayout::eUndefined) {
			description.setLoadOp(vk::AttachmentLoadOp::eClear);
		}
		else {
			description.setLoadOp(vk::AttachmentLoadOp::eLoad);
		}
		if (loadOp == vk::AttachmentLoadOp::eClear || loadOp == vk::AttachmentLoadOp::eDontCare) {
			description.setInitialLayout(vk::ImageLayout::eUndefined);
		}

	}
	static vk::AttachmentDescription StartColorInfo(vk::Format format) {

		auto info = createColorDescription(format);
		info.setInitialLayout(vk::ImageLayout::eUndefined);
		info.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		info.setLoadOp(vk::AttachmentLoadOp::eClear);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		return info;
	}
	static vk::AttachmentDescription EndColorInfo(vk::Format format) {
		auto info = createColorDescription(format);
		info.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
		info.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		info.setLoadOp(vk::AttachmentLoadOp::eLoad);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		return info;
	}
	static vk::AttachmentDescription StartDepthInfo(vk::Format format) {
		auto info = createDepthStencilDescription(format);
		info.setInitialLayout(vk::ImageLayout::eUndefined);
		//info.description.setFinalLayout(vk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
		info.setLoadOp(vk::AttachmentLoadOp::eClear);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		return info;
	}
	static vk::AttachmentDescription  EndDepthInfo(std::string name, vk::Format format) {
		auto info = createDepthStencilDescription(format);
		info.setLoadOp(vk::AttachmentLoadOp::eLoad);
		info.setStoreOp(vk::AttachmentStoreOp::eDontCare);
		return info;
	}
	struct tSubpass {
		friend class tRenderPass;
	
		tSubpass(std::vector<vk::AttachmentReference>& color, vk::AttachmentReference depth = {}, const std::vector<vk::AttachmentReference>& input_ = {}, const std::vector<vk::AttachmentReference>& resolved = {}, const std::vector<uint32_t> preserve = {}) :input(input_), color(color), depth(depth), resolved(resolved), preserve(preserve) {}
		tSubpass() {}
		bool operator==(const tSubpass& subpass) const {

			return (vkSubpassDescription == subpass.vkSubpassDescription)
				&&(input==subpass.input)
				&&(color==subpass.color)
				&&(depth==subpass.depth)
				&&(resolved==subpass.resolved);
		}
		bool operator!=(const tSubpass& subpass)const {
			return !(operator==(subpass));
		}
		void createDescription(vk::PipelineBindPoint pipelineBindPoint) {
			vkSubpassDescription.setPipelineBindPoint(pipelineBindPoint);
			vkSubpassDescription.setColorAttachments(color);
			vkSubpassDescription.setInputAttachments(input);
			vkSubpassDescription.setPDepthStencilAttachment(&depth);
			vkSubpassDescription.setPreserveAttachments(preserve);
			vkSubpassDescription.setResolveAttachments(resolved);
		}
		vk::SubpassDescription vkSubpassDescription;
		std::vector<vk::AttachmentReference> input;
		std::vector<vk::AttachmentReference> color;
		std::vector<uint32_t> preserve;
		vk::AttachmentReference depth;
		std::vector<vk::AttachmentReference> resolved;

	
	};
	
	struct tFrameBuffer;
	std::vector<vk::SubpassDependency> GetOnePassDependency();
	std::vector<vk::SubpassDependency> GetTwoPassDependency();
	class tRenderPass {
	public:
		using SharedPtr = std::shared_ptr<tRenderPass>;
		tRenderPass(uniqueDevice device):device(device) {

		}
		~tRenderPass() {
			if (vkRenderPass) {
				if (!device.expired()) {
					device.lock()->destroyRenderPass(vkRenderPass);
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}
		vk::PipelineBindPoint pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	
		
		void AddPass(tSubpass& subpass) {

			this->subpass.push_back(subpass);
		}
		void AddPass(const std::vector<vk::AttachmentReference>& colorAttachments, const vk::AttachmentReference& depthAttachment = {}) {
			tSubpass spass;
			spass.color = colorAttachments;
			spass.depth = depthAttachment;
			this->subpass.push_back(spass);
		}
		void AddDependencies(const std::vector<vk::SubpassDependency>& dependency) {
			dependencies = dependency;
		}
		void CreateRenderPass();
		uint32_t attachmentsCount() {
			return static_cast<uint32_t>(attachments.size());
		}
		vk::RenderPass vkRenderPass;
		std::vector<tSubpass> subpass;

		
		std::vector<vk::SubpassDependency> dependencies;
		
	
	protected:
	
	
		void AddFBOObserver(std::shared_ptr<tFrameBuffer> frameBuffer);
		void RemoveFBOObserver(std::shared_ptr<tFrameBuffer> frameBuffer);
		std::vector<std::weak_ptr<tFrameBuffer>> frameBufferObserver;
		void AddAttachments(std::vector<vk::AttachmentDescription> Attachments) {
			auto pushAttachmentDesc = [this](std::vector<vk::AttachmentDescription>& Attachments) {
				for (int i = 0; i < Attachments.size(); ++i) {
					UpdateEfficiency(Attachments[i]);
				}
			};

			pushAttachmentDesc(Attachments);
		}
		
	private:
		weakDevice device;
	};
	struct tFrameBuffer {

	public:
		using SharedPtr = std::shared_ptr<tFrameBuffer>;
		tFrameBuffer(uniqueDevice device) :device(device) {

		}
		~tFrameBuffer() {
			if (vkFrameBuffer) {
				if (!device.expired()) {
					device.lock()->destroyFramebuffer(vkFrameBuffer);
				}
				else {
					reportDestroyedAfterDevice();
				}
			}
		}

	
	
		vk::Rect2D RenderArea() {
			return vk::Rect2D({ 0, 0 }, { width, height });
		}

		tFrameBuffer(tRenderPass::SharedPtr renderPass_) :renderPass(renderPass_) {
			//frameBufferCreateInfo.setRenderPass(renderPass->renderPass);
			frameBufferImages.resize(renderPass->attachmentsCount());
		}
		//To do, resize frameBuffer count
		//Add big array first
		/*void SetImageForAttachment(std::string attachementName, tImage::SharedPtr image) {
			frameBufferImage[renderPass->GetAttachmentIdx(attachementName)] = image;
		}*/


		

		const tRenderPass* GetRenderPass() {
			return renderPass.get();
		}
		vk::Framebuffer vkFrameBuffer;

		tRenderPass::SharedPtr renderPass;
		std::vector<std::shared_ptr<tImageView>> frameBufferImages;
	private:
		void SetUpFrameBuffer(std::string name = "tFrameBuffer");
		
		vk::FramebufferCreateInfo frameBufferCreateInfo;//per frame resource
	
		
		uint32_t width =(uint32_t) -1;
		uint32_t height = (uint32_t)-1;
		
		
	private:
		weakDevice device;
	};
}