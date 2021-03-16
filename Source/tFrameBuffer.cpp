#include"tFrameBuffer.h"
namespace tEngine {

	void tSubpass::addColorOutput(std::string name, vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal) {
		auto& r=renderPass.getAttachment(name);
		color.emplace_back(vk::AttachmentReference(r.idx,layout));
	}
	void tSubpass::setDepth(std::string name, vk::ImageLayout layout = vk::ImageLayout::eDepthStencilAttachmentOptimal) {
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
	
}