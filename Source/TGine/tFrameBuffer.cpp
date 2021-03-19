#include"tFrameBuffer.h"
#include"tResource.h"
namespace tEngine {

	void tSubpass::addColorOutput(std::string name, vk::ImageLayout layout ) {
		auto& r=renderPass.getAttachment(name);
		color.emplace_back(vk::AttachmentReference(r.idx,layout));
	}
	void tSubpass::setDepth(std::string name, vk::ImageLayout layout) {
		auto& r = renderPass.getAttachment(name);
		depth=(vk::AttachmentReference(r.idx, layout));
	}
	void tSubpass::addResolvedOutput(std::string name, vk::ImageLayout layout) {
		auto& r = renderPass.getAttachment(name);
		resolved.emplace_back(r.idx, layout);
	}
	void tSubpass::addPreserve(std::string name) {
		auto& r = renderPass.getAttachment(name);
		resolved.emplace_back(r.idx);
	}
	FrameBufferHandle tRenderPass::requestFrameBuffer() {


		auto result = frameBuffersPool.request(views);
		if (result == nullptr) {
			result = frameBuffersPool.allocate(views, device);
			result->pass = this;
			result->setupFrameBuffer();
		}

		return result;
	}
	vk::SubpassDescription& tSubpass::createDescription(VkPipelineBindPoint pipelineBindPoint) {
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

	void tRenderPass::SetImageView(std::string name, const ImageHandle& handle, vk::ImageView view ) {

		assert(resource_to_idx.count(name) != 0);

		images[resource_to_idx[name]] = handle;
		if (view)
			views[resource_to_idx[name]] = view;
		else views[resource_to_idx[name]] = handle->get_view()->getDefaultView();

	}
}