#include"tFrameBuffer.h"
namespace tEngine {
	void tRenderPass::AddFBOObserver(std::shared_ptr<tFrameBuffer> frameBuffer) {
		frameBufferObserver.push_back(frameBuffer);
	}
	void tRenderPass::RemoveFBOObserver(std::shared_ptr<tFrameBuffer> frameBuffer) {
		for (auto& iter = frameBufferObserver.begin(); iter != frameBufferObserver.end();) {
			if (frameBuffer->vkFrameBuffer == (*iter).lock()->vkFrameBuffer) {
				frameBufferObserver.erase(iter);
				continue;
			}
			iter++;
		}
	}
	void tRenderPass::CreateRenderPass() {
		vk::RenderPassCreateInfo rpInfo;
		rpInfo.setAttachments(attachments);
		rpInfo.setDependencies(dependencies);
		std::vector<vk::SubpassDescription> subpasses(subpass.size());
		for (int i = 0; i < subpasses.size(); ++i) {
			subpass[i].createDescription(pipelineBindPoint);
			subpasses[i] = subpass[i].vkSubpassDescription;
		}
		rpInfo.setSubpasses(subpasses);
		vkRenderPass = device.lock()->createRenderPass(rpInfo);
	}
	std::vector<vk::SubpassDependency> GetOnePassDependency() {
		std::vector<vk::SubpassDependency> dependencies(2);
		dependencies[0] = vk::SubpassDependency({}, 0, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eNoneKHR, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite, vk::DependencyFlagBits::eByRegion);
		dependencies[1] = vk::SubpassDependency(0, {}, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe,
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentRead, vk::AccessFlagBits::eNoneKHR, vk::DependencyFlagBits::eByRegion);

		return dependencies;
	}
	std::vector<vk::SubpassDependency> GetTwoPassDependency() {
		std::vector<vk::SubpassDependency> dependencies(3);
		dependencies[0].setSrcSubpass({});
		dependencies[0].setDstSubpass(0);
		dependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eTopOfPipe);
		dependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependencies[0].setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);// = VK_ACCESS_MEMORY_READ_BIT;
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
}