#include"FrameBuffer.h"
#include"Buffer.h"
#include"Image.h"
#include"Device.h"
#include"Log.h"
#include"TextureFormatLayout.h"
namespace tEngine {
	void tSubpass::addInput(std::string name, vk::ImageLayout layout) {
		auto& r = renderPass.getAttachment(name,false);
		input.emplace_back(vk::AttachmentReference2(r.idx, layout, (vk::ImageAspectFlags)format_to_aspect_mask((VkFormat)r.description.format)));
	}
	void tSubpass::addColorOutput(std::string name, vk::ImageLayout layout) {
		auto& r = renderPass.getAttachment(name, false);
		color.emplace_back(vk::AttachmentReference2(r.idx, layout, (vk::ImageAspectFlags)format_to_aspect_mask((VkFormat)r.description.format)));
	}
	void tSubpass::setDepth(std::string name, vk::ImageLayout layout) {
		auto& r = renderPass.getAttachment(name, false);
		depth = (vk::AttachmentReference2(r.idx, layout, (vk::ImageAspectFlags)format_to_aspect_mask((VkFormat)r.description.format)));
	}
	void tSubpass::addResolvedOutput(std::string name, vk::ImageLayout layout) {
		auto& r = renderPass.getAttachment(name, false);
		resolved.emplace_back(r.idx, layout, (vk::ImageAspectFlags)format_to_aspect_mask((VkFormat)r.description.format));
	}
	void tSubpass::addPreserve(std::string name) {
		auto& r = renderPass.getAttachment(name, false);
		resolved.emplace_back(r.idx);
	}
	void tRenderPass::setTransientImageView(std::string name,vk::Extent2D extent){
		vk::Format format = getAttachment(name,false).description.format;
		if (extent.height == static_cast<uint32_t>(-1)) {
			assert(getImages().size()>0&&getImages()[0]);
			extent.width = getImages()[0]->get_width();
			extent.height = getImages()[0]->get_height();
		}
		auto shadowMapCreateInfo = ImageCreateInfo::transient_render_target(extent.width, extent.height, (VkFormat)format);
		/*if (sampled) {
			shadowMapCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}*/
		//If already have required image
		if (resource_to_idx.count(name) != 0) {
			auto img = getImages()[resource_to_idx.at(name)];
			if(img && getImages()[resource_to_idx.at(name)]->get_create_info() == shadowMapCreateInfo)
			return;
		}
		auto ShadowMap = createImage(device, shadowMapCreateInfo);
		this->SetImageView(name, ShadowMap, ShadowMap->get_view()->getDefaultView());
	}
	FrameBufferHandle tRenderPass::requestFrameBuffer() {

		
		auto result = frameBuffersPool.request(views);
		if (result == nullptr) {
			LOGD(LogLevel::Performance, "create frameBuffer");
			result = frameBuffersPool.allocate(views, device);
			result->pass = this;
			result->setupFrameBuffer();
		}

		return result;
	}
	vk::SubpassDescription2& tSubpass::createDescription(VkPipelineBindPoint pipelineBindPoint) {
		vkSubpassDescription.setPipelineBindPoint((vk::PipelineBindPoint)pipelineBindPoint);
		vkSubpassDescription.setInputAttachments(input);
		if (depth.layout != vk::ImageLayout::eUndefined) {
			vkSubpassDescription.setPDepthStencilAttachment(&depth);
		}
		else {
			vkSubpassDescription.pDepthStencilAttachment = nullptr;
		}
		vkSubpassDescription.setPreserveAttachments(preserve);
		vkSubpassDescription.colorAttachmentCount = color.size() + resolved.size();

		vkSubpassDescription.setPResolveAttachments(resolved.data());
		vkSubpassDescription.setPColorAttachments(color.data());
		return vkSubpassDescription;
	}

	void tRenderPass::SetImageView(std::string name, const ImageHandle& handle, vk::ImageView view) {

		assert(resource_to_idx.count(name) != 0);

		images[resource_to_idx[name]] = handle;
		if (view)
			views[resource_to_idx[name]] = view;
		else views[resource_to_idx[name]] = handle->get_view()->getDefaultView();

	}
	void tRenderPass::setupRenderPass(VkPipelineBindPoint bindp) {
		vk::RenderPassCreateInfo2 info;
		std::vector<vk::AttachmentDescription2> attachments;

		for (auto& re : resource) {
			attachments.emplace_back(re.description);
		}

		info.setAttachments(attachments);//Color attachment 没有设置成功
		info.setDependencies(dependencies);
		std::vector<vk::SubpassDescription2> subPass;
		for (auto& pa : passes) {
			auto& desc = pa->createDescription(bindp);
			subPass.emplace_back(pa->createDescription(bindp));
		}
		info.setSubpasses(subPass);
		vkRenderPass = device->createRenderPass2(info);
	
		renderFunction.resize(passes.size());
		images.resize(resource.size());
		views.resize(resource.size());
	}
	void tFrameBuffer::setupFrameBuffer() {
		std::vector<vk::ImageView> viewList;
		for (auto& frame : pass->getViews()) {
			viewList.emplace_back(frame);
		}

		width = pass->getImages()[0]->get_width();
		height = pass->getImages()[0]->get_height();
		vk::FramebufferCreateInfo info;
		info.setAttachments(viewList);
		info.setHeight(height);
		info.setLayers(pass->getImages()[0]->get_create_info().layers);
		info.setRenderPass(pass->getVkHandle());
		info.setWidth(width);
	
		vkFrameBuffer = device->createFramebuffer(info);

	}
	tRenderPass::~tRenderPass() {
		if (vkRenderPass) {
			device->destroyRenderPass(vkRenderPass);
		}
	}
	tFrameBuffer::~tFrameBuffer() {
		if (vkFrameBuffer) {

			device->destroyFramebuffer(vkFrameBuffer);

		}
	}
}