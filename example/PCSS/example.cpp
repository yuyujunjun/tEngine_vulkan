#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"TextureFormatLayout.h"
#include"renderer.h"
#include"Material.h"

#include<random>
#include"Log.h"
#include"GraphicsState.h"
#include<glm/gtx/quaternion.hpp>
#include"World.h"
using namespace tEngine;




int main() {

	
	ContextInit();
	CameraTransform c[2];
	c[0].m_windowSize = glm::u32vec2(tEngineContext::context.swapChain->getExtent().width, tEngineContext::context.swapChain->getExtent().height);
	c[1].m_windowSize = glm::u32vec2(tEngineContext::context.swapChain->getExtent().width, tEngineContext::context.swapChain->getExtent().height);


	auto camera_id = 0;
	CameraSystem cam;
	cam.setCamera(c);
	auto context = &tEngineContext::context;
	auto& device = context->device;
	tWorld world(device.get());
	world.AddSystem(&cam);
	auto shadowPass = getCollectShadowPass(device.get(), vk::Format::eR32Sfloat);
	auto shadowMapCreateInfo = ImageCreateInfo::render_target(1024, 1024, (VkFormat)vk::Format::eR32Sfloat);
	shadowMapCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	auto ShadowMap = createImage(device.get(), shadowMapCreateInfo);

	auto renderPass = getSingleRenderpass(device.get(), context->swapChain->getFormat());
	
	// = std::make_shared<GameObject_>();
	GameObject plane=GameObject_::Create();// = std::make_shared<GameObject_>();
	GameObject Light=GameObject_::Create();// = std::make_shared<GameObject_>();
	GameObject character = GameObject_::Create();
	
	///Shader
	
	//ShadowMap Shader
	auto shadowShader = tShader::Create(device.get());
	shadowShader->SetShaderModule({ "ShadowPass.vsh","ShadowPass.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });

	auto shadowMateial=std::make_shared<Material>(shadowShader.get());
	GraphicsState shadowMapState;
	shadowMapState.depthBias.depthBiasEnable = true;
	shadowMapState.depthBias.depthBiasConstantFactor = 1e5;
	shadowMapState.depthBias.depthBiasSlopeFactor = 1;
	shadowMapState.depthBias.depthBiasClamp = 0.2;
	shadowMateial->graphicsState = shadowMapState;
	//BlingPhong shader
	auto shader = tShader::Create(device.get());
	shader->SetShaderModule({ "draw.vsh","Pcss.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	character->AddComponent<MeshRenderer>(std::make_shared<Material>(shader.get()));
	plane->AddComponent<MeshRenderer>(std::make_shared<Material>(shader.get()));
	//Normal shader
	Light->AddComponent<MeshRenderer>(std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get())));
	world.RegistryMeshRenderer(character);
	world.RegistryMeshRenderer(plane);
	world.RegistryMeshRenderer(Light);
	//Post Shader
	auto postShader = tShader::Create(device.get());
	postShader->SetShaderModule({ "Quad.vsh","MainTex.fsh" }, { vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment });
	auto debugQuad = std::make_shared<Material>(postShader.get());


	
	//Mesh
	auto meshAsset = tEngine::LoadMesh("Obj/Marry.obj");
	character->AddComponent<MeshBuffer>()->setMeshUpload(meshAsset->mesh,device.get());
	plane->AddComponent<MeshBuffer>()->setMeshUpload(Mesh::UnitSquare(), device.get());
	
	plane->transform.rotation = glm::vec3(-90,0,0);
	plane->transform.scale = glm::vec3(10, 10, 10);
	{
		auto box = Mesh::UnitBox();
		Transform transform;
		transform.scale = glm::vec3(0.1, 0.1, 0.2);
		FlipFace(box);
		for (auto& b : box.vertices) {
			b.Position = transform.Matrix() * glm::vec4(b.Position.x, b.Position.y, b.Position.z, 1);
		}
		Light->AddComponent<MeshBuffer>()->setMeshUpload(box,device.get());
	}
	//Light->transform.scale = glm::vec3(.1,.1,.1);
	Light->transform.position = glm::vec3(1, 2, 2);
	Light->transform.rotation = glm::vec3(-120,0,0);
	Light->transform.scale = glm::vec3(1, 1, 1);
	float area = 1;
	
	float lightIntensity = .2;
	glm::vec3 uKd = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 uKs = glm::vec3(0.2, 0.2, 0.2);
	
	

	//Load Textue
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image =createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	
	//Config shaderInterface
	character->getComponent<MeshRenderer>()->material->SetImage("_MainTex", image);
	plane->getComponent<MeshRenderer>()->material->SetImage("_MainTex", tImage::requestWhiteImage(device.get()));

	auto lightArea = glm::vec4(-15, 15, -15, 15);
	GraphicsState debugState;

	auto uploadMaterial = [&](Renderer& obj) {
		obj.material->SetImage("_ShadowMap", ShadowMap, ShadowMap->get_view()->get_float_view(), tEngine::StockSampler::LinearShadow);
		obj.material->SetValue("uKd", uKd);
		obj.material->SetValue("uKs", uKs);
		obj.material->SetValue("lightPosArea", glm::vec4(Light->transform.position,area));
		obj.material->SetValue<float>("lightIntensity", lightIntensity);
		obj.material->SetValue("cameraPos", cam.getCameraPosition());
		obj.material->SetValue("world_to_shadow", Ortho(lightArea.x, lightArea.y, lightArea.z, lightArea.w, 40)* glm::lookAt(Light->transform.position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
		obj.material->SetValue("halfblockSize", 8);
		obj.material->SetValue("maxKernelSize",8);
		obj.material->SetValue("depthMapSize",glm::vec2(ShadowMap->get_width(),ShadowMap->get_height()) );
	};

	context->Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		if (ImGui::IsKeyPressed(GLFW_KEY_T,0)) {
			camera_id += 1;
			camera_id = camera_id % 2;
			cam.setCamera(c + camera_id);
		}
		world.update(timeDelta);
		shadowPass->SetImageView("shadowMap", ShadowMap, ShadowMap->get_view()->getDefaultView());
		shadowPass->setTransientImageView("depth");
		//renderPass->SetImageView("shadow", ShadowMap, ShadowMap->get_view()->get_float_view());
		renderPass->SetImageView("back", context->swapChain->getImage(context->getImageIdx()));
		renderPass->setTransientImageView("depth");
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		
		uploadMaterial(*character->getComponent<MeshRenderer>());
		uploadMaterial(*plane->getComponent<MeshRenderer>());
		
		ImGui::ShowDemoWindow();
		ImGui::Begin("test");
		ImGui::DragFloat3("lightPos",&Light->transform.position[0],0.1,-10,10);
		ImGui::DragFloat("lightArea", &area, 0.1, 0.01, 30);
		ImGui::DragFloat3("lightRotation", &Light->transform.rotation[0],1,-360,360);
		ImGui::InputFloat3("lightScale", &Light->transform.scale[0]);
		ImGui::InputFloat4("area", &lightArea[0]);
		ImGui::DragFloat3("uKd",&uKd[0],.01,0,3);
		ImGui::DragFloat3("uKs",&uKs[0],.01,0,15);
		ImGui::DragFloat("lightIntensity",&lightIntensity,.01,0,10);

		
		ImGui::End();
	

		});
	context->Record([&](double timeDelta, CommandBufferHandle& cb) {
			auto depthFB = shadowPass->requestFrameBuffer();
			cb->beginRenderPass(shadowPass, depthFB,true);

			cb->setViewport(depthFB->getViewPort());
			cb->setScissor(0, depthFB->getRenderArea());
			//Update shadowmap
			auto cameraBufferRange=requestCameraBufferRange(device.get());
			cameraBufferRange->NextRangenoLock();
			auto view = (glm::lookAt(Light->transform.position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
			auto projection = Ortho(lightArea.x, lightArea.y, lightArea.z, lightArea.w, 40);
			uploadCameraMatrix(view, projection, cameraBufferRange->buffer().get(), cameraBufferRange->getOffset());
			shadowMateial->SetBuffer("CameraMatrix", cameraBufferRange->buffer(), cameraBufferRange->getOffset());
			RenderInfo renderInfo;
			renderInfo.renderPass = shadowPass.get();
			renderInfo.subpass = 0;
			RenderWithMaterial(cb, renderInfo, world.getRenderers(), shadowMateial.get());
			cb->endRenderPass();
			
			auto& frameBuffer = renderPass->requestFrameBuffer();
			
			auto viewPort = frameBuffer->getViewPort();
			cb->beginRenderPass(renderPass, frameBuffer, true);
			cb->setViewport(frameBuffer->getViewPort());
			cb->setScissor(0,frameBuffer->getRenderArea());
			RenderInfo info;
			info.renderPass = renderPass.get();
			info.subpass = 0;
			world.renderWithCamera(cb, info, &c[camera_id]);
			debugQuad->SetImage("_MainTex",ShadowMap,ShadowMap->get_view()->get_float_view(),tEngine::StockSampler::NearestShadow);
			flushGraphicsShaderState(debugQuad->shader,debugState, cb, renderPass.get(), 0);
			
			viewPort.height /= 3;
			viewPort.width /= 3;
		
			auto area = frameBuffer->getRenderArea();
			area.extent.height /= 3;
			area.extent.width /= 3;

			cb->setViewport(viewPort);
			cb->setDepthBias(0.1,0.2,10);
			cb->setScissor(0, frameBuffer->getRenderArea());
			DrawMesh(plane->getComponent<MeshBuffer>(),cb,1);
			cb->endRenderPass();
			setImageLayout(cb, ShadowMap, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthAttachmentOptimal);

		
		});

	
	auto threadContext = context->AddThreadContext();
	context->Loop(threadContext);


	


	
	
	
	
	//shader.reset();
	
	//image.reset();
	//CameraBuffer.removeBuffer();
	//modelMatrix.removeBuffer();
	//renderPass.reset();

	//delete context;
	//shaderInterface.reset();
	//CreateImageViewWithImage();

	return 0;
}