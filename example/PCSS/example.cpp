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
	auto context = &tEngineContext::context;
	auto& device = context->device;
	auto cameraObj = Camera::Create();
	auto cam=cameraObj->getComponent<Camera>();
	cam->transform.m_windowSize = glm::uvec2(context->swapChain->getExtent().width, context->swapChain->getExtent().height);
	auto camera_id = 0;
	CameraSystem camSys;
	camSys.setCamera(&cam->transform);
	tWorld world(device.get());
	world.AddSystem(&camSys);
	auto shadowPass = getCollectShadowPass(device.get(), vk::Format::eR32Sfloat);
	auto shadowMapCreateInfo = ImageCreateInfo::render_target(1024, 1024, (VkFormat)vk::Format::eR32Sfloat);
	shadowMapCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	auto ShadowMap = createImage(device.get(), shadowMapCreateInfo);
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image = createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	auto shadowShader = tShader::Create(device.get());
	shadowShader->SetShaderModule({ "ShadowPass.vsh","ShadowPass.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto shadowMateial = std::make_shared<Material>(shadowShader.get());
	GraphicsState shadowMapState;
	shadowMapState.depthBias.depthBiasEnable = true;
	shadowMapState.depthBias.depthBiasConstantFactor = 1e5;
	shadowMapState.depthBias.depthBiasSlopeFactor = 1;
	shadowMapState.depthBias.depthBiasClamp = 0.2;
	shadowMateial->graphicsState = shadowMapState;
	//Post Shader
	auto postShader = tShader::Create(device.get());
	postShader->SetShaderModule({ "Quad.vsh","MainTex.fsh" }, { vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment });
	//BlingPhong shader
	auto shader = tShader::Create(device.get());
	shader->SetShaderModule({ "draw.vsh","Pcss.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	//Mesh
	auto meshAsset = tEngine::LoadMesh("Obj/Marry.obj");
	auto box = Mesh::UnitBox();
	Transform transform;
	transform.setScale(glm::vec3(0.1, 0.1, 0.2));
	FlipFace(box);
	for (auto& b : box.vertices) {
		b.Position = transform.updateMtx() * glm::vec4(b.Position.x, b.Position.y, b.Position.z, 1);
	}
	// = std::make_shared<GameObject_>();
	GameObject plane=GameObject_::Create();// = std::make_shared<GameObject_>();
	GameObject Light=GameObject_::Create();// = std::make_shared<GameObject_>();
	GameObject character = GameObject_::Create();
	GameObject debugQuat = GameObject_::Create();
	character->AddComponent<MeshRenderer>(std::make_shared<Material>(shader.get()));
	plane->AddComponent<MeshRenderer>(std::make_shared<Material>(shader.get()));
	Light->AddComponent<MeshRenderer>(std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get())));
	Light->getComponent<MeshRenderer>()->shadowCastingMode = Renderer::ShadowCastingMode::Off;//avoid light object to cast shadow
	debugQuat->AddComponent<MeshRenderer>(std::make_shared<Material>(postShader.get()));
	debugQuat->getComponent<MeshRenderer>()->setLayer(RenderLayer::UI);//avoid debugQuat to be rendered by a common render
	debugQuat->getComponent<MeshRenderer>()->material->SetImage("_MainTex", ShadowMap, ShadowMap->get_view()->get_float_view(), tEngine::StockSampler::NearestShadow);
	//Config shaderInterface
	character->getComponent<MeshRenderer>()->material->SetImage("_MainTex", image);
	plane->getComponent<MeshRenderer>()->material->SetImage("_MainTex", tImage::requestWhiteImage(device.get()));
	plane->transform.setOrientation(glm::vec3(-90, 0, 0));
	plane->transform.setScale(glm::vec3(10, 10, 10));
	Light->transform.setPosition(glm::vec3(0, 6, 10));
	Light->transform.setOrientation(glm::vec3(-120, 0, 0));
	Light->transform.setScale(glm::vec3(1, 1, 1));
	float area = 5;
	float lightIntensity = .2;
	glm::vec3 uKd = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 uKs = glm::vec3(0.2, 0.2, 0.2);
	character->AddComponent<MeshFilter>()->setMeshUpload(meshAsset->mesh, device.get());
	plane->AddComponent<MeshFilter>()->setMeshUpload(Mesh::UnitSquare(), device.get());
	Light->AddComponent<MeshFilter>()->setMeshUpload(box, device.get());
	debugQuat->AddComponent<MeshFilter>()->setMeshUpload(Mesh::UnitSquare(), device.get());
	auto lightArea = glm::vec4(-10, 10, -5, 5);
	world.AddGameObject(cameraObj);
	world.AddGameObject(character);
	world.AddGameObject(plane);
	world.AddGameObject(Light);
	auto uploadMaterial = [&](Renderer& obj) {
		obj.material->SetImage("_ShadowMap", ShadowMap, ShadowMap->get_view()->get_float_view(), tEngine::StockSampler::LinearShadow);
		obj.material->SetValue("uKd", uKd);
		obj.material->SetValue("uKs", uKs);
		obj.material->SetValue("lightPosArea", glm::vec4(Light->transform.getPosition(),area));
		obj.material->SetValue<float>("lightIntensity", lightIntensity);
		obj.material->SetValue("cameraPos", cam->transform.m_cameraPosition);
		obj.material->SetValue("world_to_shadow", Ortho(lightArea.x, lightArea.y, lightArea.z, lightArea.w, 50)* glm::lookAt(Light->transform.getPosition(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
		obj.material->SetValue("halfblockSize", 8);
		obj.material->SetValue("maxKernelSize",8);
		obj.material->SetValue("depthMapSize",glm::vec2(ShadowMap->get_width(),ShadowMap->get_height()) );
	};
	context->Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		
		world.update(timeDelta);
		shadowPass->SetImageView("back", ShadowMap, ShadowMap->get_view()->getDefaultView());
		shadowPass->setTransientImageView("depth");
		//renderPass->SetImageView("shadow", ShadowMap, ShadowMap->get_view()->get_float_view());
		uploadMaterial(*character->getComponent<MeshRenderer>());
		uploadMaterial(*plane->getComponent<MeshRenderer>());
		ImGui::ShowDemoWindow();
		ImGui::Begin("test");
		ImGui::DragFloat3("lightPos",&Light->transform.getLocalPosition()[0],0.1,-10,10);
		ImGui::DragFloat("lightArea", &area, 0.1, 0.01, 30);
		glm::vec3 eulerAngle = Light->transform.getLocalEulerAngle();
		ImGui::DragFloat3("lightRotation", &eulerAngle[0],1,-360,360);
		Light->transform.setOrientation(eulerAngle);
		ImGui::InputFloat3("lightScale", &Light->transform.getLocalScale()[0]);
		ImGui::InputFloat4("area", &lightArea[0]);
		ImGui::DragFloat3("uKd",&uKd[0],.01,0,3);
		ImGui::DragFloat3("uKs",&uKs[0],.01,0,15);
		ImGui::DragFloat("lightIntensity",&lightIntensity,.01,0,10);
		ImGui::End();
	

		});
	cam->setAfterRender([&](CommandBufferHandle cb, const BufferRangeManager* cameraBuffer,const RenderInfo& info,const FrameBufferHandle& frameBuffer) {
		auto renderPass=cam->getRenderPass();
		auto viewPort = frameBuffer->getViewPort();
		viewPort.height /= 3;
		viewPort.width /= 3;
		auto area = frameBuffer->getRenderArea();
		area.extent.height /= 3;
		area.extent.width /= 3;
		cb->setViewport(viewPort);
		cb->setDepthBias(0.1, 0.2, 10);
		cb->setScissor(0, frameBuffer->getRenderArea());
		debugQuat->getComponent<MeshRenderer>()->Draw(cb, info);
		});
	context->Record([&](double timeDelta, CommandBufferHandle& cb) {
		//Collect Shadow map
			auto depthFB = shadowPass->requestFrameBuffer();
			cb->beginRenderPass(shadowPass, depthFB,true);
			cb->setViewport(depthFB->getViewPort());
			cb->setScissor(0, depthFB->getRenderArea());
			auto view = (glm::lookAt(Light->transform.getPosition(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
			auto projection = Ortho(lightArea.x, lightArea.y, lightArea.z, lightArea.w, 50);
			RenderInfo renderInfo;
			renderInfo.renderPass = shadowPass.get();
			renderInfo.subpass = 0;
			renderInfo.isRender = [](Renderer* render)->bool {
				return (render->shadowCastingMode == Renderer::ShadowCastingMode::On);
			};
			auto cameraBuffer=UpdateCameraBuffer(device.get(),view, projection);
			world.getRenderWorld().RenderWithMaterial(cb, renderInfo, shadowMateial.get(), cameraBuffer);
			cb->endRenderPass();
			//Common Render
			world.getRenderWorld().Render(cb, context->swapChain, context->getImageIdx());
			setImageLayout(cb, ShadowMap, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthAttachmentOptimal);

		
		});

	
	auto threadContext = context->AddThreadContext();
	context->Loop(threadContext);

	return 0;
}