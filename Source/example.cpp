#include"tEngineContext.h"
using namespace tEngine;
tEngineContext tEngineContext::context;
int main() {
	
	const int width = 600;
	const int height = 800;
	
	auto& device = tEngineContext::context.device;
	ThreadContext* threadContext = new ThreadContext(device.get());
	//auto meshAsset=tEngine::LoadMesh("pig.obj");


	//Shader
	auto shader = tShader::Create(device.get());
	std::vector<std::string> shaders = { "Quad.vsh","MainTex.fsh" };
	shader->SetShaderModule(shaders, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto material = shader->getInterface();
	auto renderPass = getSingleRenderpass(device.get());

	//Load Mesh
	auto mesh = std::make_shared<MeshBuffer>();
	auto meshAsset = tEngine::LoadMesh("pig.obj");
	auto quad = Mesh::UnitSquare();
	FlipFace(quad);
	FlipFace(quad);
	mesh->setMeshUpload(quad, device.get());

	//Load Textue
	auto imageAsset = tEngine::LoadImage("174.png");
	auto image = device->createImage(
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);

	//device->createImage(ImageCreateInfo::depth());
	auto depth = device->createImage(ImageCreateInfo::render_target(800, 800, (VkFormat)vk::Format::eD16Unorm));
	//auto image2 = device->createImage(ImageCreateInfo::render_target(800, 800, (VkFormat)vk::Format::eR8G8B8A8Srgb));
	material->SetImage("_MainTex", image);
	FenceHandle fence = device->getFenceManager()->requestSingaledFence();
	auto acquireSemaphore = device->getSemaphoreManager()->requestSemaphore();
	auto presentSemaphore = device->getSemaphoreManager()->requestSemaphore();
	while (!glfwWindowShouldClose(device->getWindow())) {
		vk::ResultValue<uint32_t> currentBuffer = device->acquireNextImageKHR(device->swapChain->getVkHandle(), -1, acquireSemaphore->getVkHandle());
		uint32_t imageIdx = currentBuffer.value;
		renderPass->SetImageView("back", device->swapChain->getImage(currentBuffer.value));
		renderPass->SetImageView("depth", depth);
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		auto& cb = threadContext->cmdBuffers[imageIdx];
	
		auto result = device->waitForFences(fence->getVkHandle(), true, 100000000);

		device->resetFences(fence->getVkHandle());
		cb->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
		cb->begin(vk::CommandBufferUsageFlags());

		auto& frameBuffer = renderPass->requestFrameBuffer();
		cb->beginRenderPass(renderPass, frameBuffer, true);
		cb->setViewport(frameBuffer->getViewPort());
		cb->setScissor(0, frameBuffer->getRenderArea());
		flushGraphicsShaderState(*material.get(), cb, renderPass.get(), 0);
		DrawMesh(mesh.get(), cb);

		cb->endRenderPass();
		cb->end();

		tSubmitInfo info;
		info.setCommandBuffers(threadContext->cmdBuffers[imageIdx]);
		info.waitSemaphore(acquireSemaphore, vk::PipelineStageFlagBits::eFragmentShader);
		
		info.signalSemaphore(presentSemaphore);
	
		device->requestQueue(cb->getQueueFamilyIdx()).submit(info.getSubmitInfo(),fence->getVkHandle());
	

		device->requestQueue(vk::QueueFlagBits::eGraphics).presentKHR(vk::PresentInfoKHR(presentSemaphore->getVkHandle(), device->swapChain->getVkHandle(), currentBuffer.value));
		
		glfwPollEvents();

	}


	device->waitForFences(fence->getVkHandle(),true,-1);
	threadContext->cmdBuffers[0]->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	
	delete threadContext;
	shader.reset();
	material.reset();
	mesh.reset();
	image.reset();

	//material.reset();
	//CreateImageViewWithImage();


}