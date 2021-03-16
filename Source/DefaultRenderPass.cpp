#include"tFrameBuffer.h"
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


	//Inspired by Nvidia tips
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
	static vk::AttachmentDescription StartColorDescription(vk::Format format) {

		auto info = createColorDescription(format);
		info.setInitialLayout(vk::ImageLayout::eUndefined);
		info.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		info.setLoadOp(vk::AttachmentLoadOp::eClear);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		return info;
	}
	static vk::AttachmentDescription EndColorDescription(vk::Format format) {
		auto info = createColorDescription(format);
		info.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal);
		info.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		info.setLoadOp(vk::AttachmentLoadOp::eLoad);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		return info;
	}
	static vk::AttachmentDescription StartDepthDescription(vk::Format format) {
		auto info = createDepthStencilDescription(format);
		info.setInitialLayout(vk::ImageLayout::eUndefined);
		//info.description.setFinalLayout(vk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
		info.setLoadOp(vk::AttachmentLoadOp::eClear);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		return info;
	}
	static vk::AttachmentDescription  EndDepthDescription(std::string name, vk::Format format) {
		auto info = createDepthStencilDescription(format);
		info.setLoadOp(vk::AttachmentLoadOp::eLoad);
		info.setStoreOp(vk::AttachmentStoreOp::eDontCare);
		return info;
	}
	static vk::AttachmentDescription SingleDepthDescription(vk::Format format) {
		auto info = createDepthStencilDescription(format);
		info.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		info.setLoadOp(vk::AttachmentLoadOp::eClear);
		info.setStoreOp(vk::AttachmentStoreOp::eDontCare);
		return info;

	}
	std::vector<vk::SubpassDependency> SingleDependencies() {
		std::vector<vk::SubpassDependency> dependencies(2);
		dependencies[0] = vk::SubpassDependency({}, 0, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::DependencyFlagBits::eByRegion);
		dependencies[1] = vk::SubpassDependency(0, {}, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentRead, vk::AccessFlagBits::eNoneKHR, vk::DependencyFlagBits::eByRegion);

		return dependencies;
	}
	std::vector<vk::SubpassDependency> WriteBeforeShaderReadDependencies() {
		std::vector<vk::SubpassDependency> dependencies(3);
		dependencies[0].setSrcSubpass({});
		dependencies[0].setDstSubpass(0);
		dependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eTopOfPipe);
		dependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependencies[0].setSrcAccessMask(vk::AccessFlagBits::eNoneKHR);// = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);// = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

		dependencies[1].setSrcSubpass(0);
		dependencies[1].setDstSubpass(1);
		dependencies[1].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependencies[1].setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
		dependencies[1].setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		dependencies[1].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		dependencies[1].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

		dependencies[2].setSrcSubpass(1);
		dependencies[2].setDstSubpass({});
		dependencies[2].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependencies[2].setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
		dependencies[2].setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		dependencies[2].setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
		dependencies[2].setDependencyFlags(vk::DependencyFlagBits::eByRegion);
		return dependencies;


	}
	RenderPassHandle getSingleRenderpass(Device* device) {
		auto& handle = std::make_shared<tRenderPass>(device);
		auto& pass= handle->getPass("Single");
		pass.addColorOutput("back", (vk::ImageLayout)VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		pass.setDepth("Depth",(vk::ImageLayout)VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		handle->SetAttachmentDescription("back", StartColorDescription(device->swapChain->getFormat()));
		handle->SetAttachmentDescription("Depth", SingleDepthDescription((vk::Format)default_depth_stencil_format(device->getPhysicalDevice().physicalDevice)));
		handle->setupRenderPass();
		return handle;
	}
}