#include"renderHelper.h"
#include"Device.h"
#include"SwapChain.h"
#include"FrameBuffer.h"
namespace tEngine {
	inline vk::AttachmentDescription2 createDepthStencilDescription(vk::Format format, vk::ImageLayout initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
		vk::ImageLayout finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare, vk::SampleCountFlagBits numSamples = vk::SampleCountFlagBits::e1)
	{
		vk::AttachmentDescription2 attach = { {}, format, numSamples, loadOp, storeOp, stencilLoadOp, stencilStoreOp, initialLayout, finalLayout };
		return attach;
	}
	inline vk::AttachmentDescription2 createColorDescription(vk::Format format, vk::ImageLayout initialLayout = vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore, vk::SampleCountFlagBits numSamples = vk::SampleCountFlagBits::e1)
	{
		vk::AttachmentDescription2 attach = { {}, format, numSamples, loadOp, storeOp, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, initialLayout, finalLayout };
		return attach;
	}


	//Inspired by Nvidia tips
	//Use VK_IMAGE_LAYOUT_UNDEFINED when the previous content of the image is not needed.
	inline void UpdateEfficiency(vk::AttachmentDescription2& description) {
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
	static vk::AttachmentDescription2 StartColorDescription(vk::Format format) {

		auto info = createColorDescription(format);
		info.setInitialLayout(vk::ImageLayout::eUndefined);
		info.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		info.setLoadOp(vk::AttachmentLoadOp::eClear);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		return info;
	}
	static vk::AttachmentDescription2 EndColorDescription(vk::Format format) {
		auto info = createColorDescription(format);
		info.setInitialLayout(vk::ImageLayout::ePresentSrcKHR);
		info.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
		info.setLoadOp(vk::AttachmentLoadOp::eLoad);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		return info;
	}
	static vk::AttachmentDescription2 StartDepthDescription(vk::Format format) {
		auto info = createDepthStencilDescription(format);
		info.setInitialLayout(vk::ImageLayout::eUndefined);
		//info.description.setFinalLayout(vk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL);
		info.setLoadOp(vk::AttachmentLoadOp::eClear);
		info.setStoreOp(vk::AttachmentStoreOp::eStore);
		
		return info;
	}
	static vk::AttachmentDescription2  EndDepthDescription( vk::Format format) {
		auto info = createDepthStencilDescription(format);
		info.setLoadOp(vk::AttachmentLoadOp::eLoad);
		info.setStoreOp(vk::AttachmentStoreOp::eDontCare);
		return info;
	}
	static vk::AttachmentDescription2 SingleDepthDescription(vk::Format format) {
		auto info = createDepthStencilDescription(format);
		info.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		info.setLoadOp(vk::AttachmentLoadOp::eClear);
		info.setStoreOp(vk::AttachmentStoreOp::eDontCare);
		return info;

	}
	std::vector<vk::SubpassDependency2> SingleDependencies() {
		std::vector<vk::SubpassDependency2> dependencies(2);
		dependencies[0] = vk::SubpassDependency2({}, 0, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::DependencyFlagBits::eByRegion);
		dependencies[1] = vk::SubpassDependency2(0, {}, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentRead, vk::AccessFlagBits::eNoneKHR, vk::DependencyFlagBits::eByRegion);

		return dependencies;
	}
	std::vector<vk::SubpassDependency2> WriteBeforeShaderReadDependencies() {
		std::vector<vk::SubpassDependency2> dependencies(3);
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
	RenderPassHandle getSingleRenderpass(Device* device, vk::Format format, vk::Format depthFormat) {
		auto& handle = std::make_shared<tRenderPass>(device);
		//	pass.addColorOutput("debug", (vk::ImageLayout)VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		handle->SetAttachmentDescription("back", StartColorDescription(format));
		//	handle->SetAttachmentDescription("debug", StartColorDescription(vk::Format::eR8G8B8A8Srgb));
		handle->SetAttachmentDescription("depth", StartDepthDescription(depthFormat));
		auto& pass = handle->getPass("Single");
		pass.addColorOutput("back", (vk::ImageLayout)VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		pass.setDepth("depth", vk::ImageLayout::eDepthStencilAttachmentOptimal);

		handle->SetDependencies(SingleDependencies());
		handle->setClearValue("back", {0,0,0,0});
		handle->setDepthStencilValue("depth",1,0);
		handle->setupRenderPass();
		return handle;
	}
	RenderPassHandle getUIRenderpass(Device* device, vk::Format format) {
		auto& handle = std::make_shared<tRenderPass>(device);
	
	//	pass.setDepth("depth", vk::ImageLayout::eDepthStencilAttachmentOptimal);
		//	pass.addColorOutput("debug", (vk::ImageLayout)VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		handle->SetAttachmentDescription("back", EndColorDescription(format));
		//	handle->SetAttachmentDescription("debug", StartColorDescription(vk::Format::eR8G8B8A8Srgb));
		//handle->SetAttachmentDescription("depth", EndDepthDescription(depthFormat));
		auto& pass = handle->getPass("Single");
		pass.addColorOutput("back", (vk::ImageLayout)VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		handle->SetDependencies(SingleDependencies());
		handle->setClearValue("back", { 0,0,0,0 });
	//	handle->setDepthStencilValue("depth", 1, 0);
		handle->setupRenderPass();
		return handle;
	}
	RenderPassHandle getCollectShadowPass(Device* device, vk::Format format) {
		auto& handle = std::make_shared<tRenderPass>(device);
		auto desc=StartColorDescription(format);
		desc.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		handle->SetAttachmentDescription("shadowMap", desc);
		handle->SetAttachmentDescription("depth", StartDepthDescription((vk::Format)default_depth_format(device->getPhysicalDevice().physicalDevice)));
		auto& pass = handle->getPass("CollectShadow");
		pass.addColorOutput("shadowMap", vk::ImageLayout::eColorAttachmentOptimal);
		pass.setDepth("depth", vk::ImageLayout::eDepthStencilAttachmentOptimal);
		handle->SetDependencies(SingleDependencies());
		//handle->setClearValue("shadowMap", { 0,0,0,0 });
		handle->setDepthStencilValue("depth", 1.);
		handle->setupRenderPass();
		return handle;
	}
	RenderPassHandle getShadowMapPass(Device* device, vk::Format format, vk::Format depthFormat) {
		auto& handle = std::make_shared<tRenderPass>(device);
		auto shadowDesc = StartDepthDescription(depthFormat);
		shadowDesc.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		handle->SetAttachmentDescription("shadow", shadowDesc);
		handle->SetAttachmentDescription("back", StartColorDescription(format));
		handle->SetAttachmentDescription("depth", StartDepthDescription(depthFormat));

		auto& shadow_pass = handle->getPass("ShadowMap");
		shadow_pass.setDepth("shadow", vk::ImageLayout::eDepthStencilAttachmentOptimal);

		auto& pass = handle->getPass("Color");
		pass.addColorOutput("back", (vk::ImageLayout)VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		pass.setDepth("depth", vk::ImageLayout::eDepthStencilAttachmentOptimal);
		pass.addInput("shadow", vk::ImageLayout::eShaderReadOnlyOptimal);
		


		
		
		
	
		
	
	

		std::vector<vk::SubpassDependency2> dependencies(3);
		dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependencies[0].setDstSubpass(0);
		dependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eTopOfPipe);
		dependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eLateFragmentTests);
		dependencies[0].setSrcAccessMask(vk::AccessFlagBits::eNoneKHR);// = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);// = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

		dependencies[1].setSrcSubpass(0);
		dependencies[1].setDstSubpass(1);
		dependencies[1].setSrcStageMask(vk::PipelineStageFlagBits::eLateFragmentTests);
		dependencies[1].setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader);
		dependencies[1].setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		dependencies[1].setDstAccessMask(vk::AccessFlagBits::eShaderRead);
		dependencies[1].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

		dependencies[2].setSrcSubpass(1);
		dependencies[2].setDstSubpass(VK_SUBPASS_EXTERNAL);
		dependencies[2].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependencies[2].setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
		dependencies[2].setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		dependencies[2].setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
		dependencies[2].setDependencyFlags(vk::DependencyFlagBits::eByRegion);


		handle->SetDependencies(dependencies);
		handle->setClearValue("back", { 0,0,0,0 });
		handle->setDepthStencilValue("depth", 1, 0);
		handle->setDepthStencilValue("shadow", 1, 0);
		handle->setupRenderPass();
		return handle;
	}
}