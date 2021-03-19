#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
using namespace tEngine;
CameraManipulator cameraManipulator;
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	CameraManipulator::MouseButton mouseButton =
		(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		?  CameraManipulator::MouseButton::Left
		: (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		?  CameraManipulator::MouseButton::Middle
		: (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		?  CameraManipulator::MouseButton::Right
		:  CameraManipulator::MouseButton::None;
	if (mouseButton !=  CameraManipulator::MouseButton::None)
	{
		CameraManipulator::ModifierFlags modifiers=0;
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
		{
			modifiers |=  static_cast<uint32_t>(CameraManipulator::ModifierFlagBits::Alt);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			modifiers |= static_cast<uint32_t>(CameraManipulator::ModifierFlagBits::Ctrl);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			modifiers |= static_cast<uint32_t>(CameraManipulator::ModifierFlagBits::Shift);
		}

		
		cameraManipulator.mouseMove(
			glm::ivec2(static_cast<int>(xpos), static_cast<int>(ypos)), mouseButton, modifiers);
	}
}
static void mouseButtonCallback(GLFWwindow* window, int /*button*/, int /*action*/, int /*mods*/)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	cameraManipulator.setMousePosition(glm::ivec2(static_cast<int>(xpos), static_cast<int>(ypos)));
}
static void scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{

	cameraManipulator.wheel(static_cast<int>(yoffset));
}
int main() {
	const size_t xMax = 3;
	const size_t yMax = 3;
	const size_t zMax = 3;
	const int width = 1200;
	const int height = 800;
	tEngineContext context(vk::Extent2D(width,height));
	auto& device = context.device;
	glfwSetCursorPosCallback(device->getWindow(), cursor_position_callback);
	glfwSetMouseButtonCallback(device->getWindow(), mouseButtonCallback);
	glfwSetScrollCallback(device->getWindow(), scrollCallback);
	ThreadContext* threadContext = new ThreadContext(device.get());
	//auto meshAsset=tEngine::LoadMesh("pig.obj");

	cameraManipulator.setWindowSize(glm::u32vec2(width, height));
	glm::vec3 diagonal =
		3.0f * glm::vec3(static_cast<float>(xMax), static_cast<float>(yMax), static_cast<float>(zMax));
	cameraManipulator.setLookat(1.5f * diagonal, 0.5f * diagonal, glm::vec3(0, 1, 0));
	//Shader
	auto shader = tShader::Create(device.get());
	std::vector<std::string> shaders = { "draw.vsh","draw.fsh" };
	shader->SetShaderModule(shaders, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto material = shader->getInterface();
	auto renderPass = getSingleRenderpass(device.get());

	//Load Mesh
	auto mesh = std::make_shared<MeshBuffer>();
	auto meshAsset = tEngine::LoadMesh("pig.obj");
	auto quad = Mesh::UnitSquare();
	//FlipFace(quad);
	FlipFace(quad);
	mesh->setMeshUpload(meshAsset->mesh, device.get());

	//Load Textue
	auto imageAsset = tEngine::LoadImage("174.png");
	auto image =createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	auto depth = createImage(device.get(),ImageCreateInfo::render_target(width, height, (VkFormat)vk::Format::eD16Unorm));
	material->SetImage("_MainTex", image);
	FenceHandle fence = device->getFenceManager()->requestSingaledFence();
	auto acquireSemaphore = device->getSemaphoreManager()->requestSemaphore();
	auto presentSemaphore = device->getSemaphoreManager()->requestSemaphore();

	CameraManipulator camera;
	auto CameraBuffer = material->getShader()->requestBufferRange("CameraMatrix", 1);
	auto modelMatrix = material->getShader()->requestBufferRange("ModelMatrix", 1);
	material->SetBuffer("CameraMatrix", CameraBuffer.buffer(), 3);
	material->SetBuffer("ModelMatrix", modelMatrix.buffer(), 10);


	while (!glfwWindowShouldClose(device->getWindow())) {
		vk::ResultValue<uint32_t> currentBuffer = device->acquireNextImageKHR(device->swapChain->getVkHandle(), -1, acquireSemaphore->getVkHandle());
		uint32_t imageIdx = currentBuffer.value;
		renderPass->SetImageView("back", device->swapChain->getImage(currentBuffer.value));
		renderPass->SetImageView("depth", depth);
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);

		CameraBuffer.NextRangenoLock();
		modelMatrix.NextRangenoLock();
		material->SetBuffer("CameraMatrix", CameraBuffer.buffer(), CameraBuffer.getOffset());
		material->SetBuffer("ModelMatrix", modelMatrix.buffer(), modelMatrix.getOffset());
	//	std::cout << cameraManipulator.getMatrix() << std::endl;
		uploadCameraMatrix(cameraManipulator.getMatrix(),Perspective(device->swapChain->getExtent()),material.get());
		material->SetValue(ShaderString(SV::_MATRIX_M), glm::mat4(1));
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
//	threadContext->cmdBuffers[0]->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	
	delete threadContext;
	shader.reset();
	material.reset();
	mesh.reset();
	image.reset();

	//material.reset();
	//CreateImageViewWithImage();


}