#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"TextureFormatLayout.h"
#include"renderer.h"
#include"Material.h"

#include<random>
#include"Log.h"
#include"GraphicsState.h"
#include<glm/gtx/quaternion.hpp>
#include"World.h"
#include"Light.h"
using namespace tEngine;




int main() {

	
	ContextInit();
	auto context = &tEngineContext::context;
	
	auto& device = context->device;
	tWorld world(device.get());

	auto scene =&world.getEcsManager();
	auto camera = createCamera(&world.getEcsManager(), glm::uvec2(context->swapChain->getExtent().width, context->swapChain->getExtent().height));
	world.getRenderWorld().SetCamera(camera);
	
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image = createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	

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

	auto  Light = createGameObject(scene);
	auto shadowMap = scene->AddComponent<tEngine::Light>(Light,device.get())->shadowMap;
	scene->GetComponent<View>(Light)->interactShadow(false);
	scene->GetComponent<View>(Light)->material = std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get()));
	scene->GetComponent<MeshFilter>(Light)->setMeshUpload(box, device.get());
	scene->GetComponent<Transform>(Light)->setPosition(glm::vec3(0, 6, 10));
	scene->GetComponent<Transform>(Light)->setOrientation(glm::vec3(-120, 0, 0));
	scene->GetComponent<Transform>(Light)->setScale(glm::vec3(1, 1, 1));
	
	auto debugQuat = createGameObject(scene);
	scene->GetComponent<MeshFilter>(debugQuat)->setMeshUpload(Mesh::UnitSquare(), device.get());
	scene->GetComponent<View>(debugQuat)->interactShadow(false);
	scene->GetComponent<View>(debugQuat)->renderOrder = 2;
	scene->GetComponent<View>(debugQuat)->layer = RenderLayer::UI;
	scene->GetComponent<View>(debugQuat)->material = std::make_shared<Material>(postShader.get());
	scene->GetComponent<View>(debugQuat)->material->SetImage("_MainTex", shadowMap, shadowMap->get_view()->get_float_view(), tEngine::StockSampler::NearestShadow);
	
	
	auto character = createGameObject(scene);
	scene->GetComponent<View>(character)->material = std::make_shared<Material>(shader.get());
	scene->GetComponent<View>(character)->material->SetImage("_MainTex", image);
	scene->GetComponent<MeshFilter>(character)->setMeshUpload(meshAsset->mesh, device.get());
	scene->GetComponent<View>(character)->interactShadow(true);
	
	
	auto plane = createGameObject(scene);
	scene->GetComponent<View>(plane)->material = std::make_shared<Material>(shader.get());
	scene->GetComponent<View>(plane)->material->SetImage("_MainTex", tImage::requestWhiteImage(device.get()));
	scene->GetComponent<MeshFilter>(plane)->setMeshUpload(Mesh::UnitSquare(), device.get());
	scene->GetComponent<Transform>(plane)->setOrientation(glm::vec3(-90, 0, 0));
	scene->GetComponent<Transform>(plane)->setScale(glm::vec3(10, 10, 10));

	scene->GetComponent<View>(plane)->interactShadow(true);

	//float lightIntensity = .2;
	glm::vec3 uKd = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 uKs = glm::vec3(0.2, 0.2, 0.2);
	auto lightArea = glm::vec4(-10, 10, -5, 5);
	auto uploadMaterial = [&](View* obj) {
		
		obj->material->SetValue("uKd", uKd);
		obj->material->SetValue("uKs", uKs);
	
	};
	CameraSystem cam_sys;
	cam_sys.ecsManager = scene;
	cam_sys.setCamera(camera);
	context->Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		cam_sys.ExecuteAllComponents(timeDelta);
		world.update(timeDelta);
		
		uploadMaterial(scene->GetComponent<View>(character));
		uploadMaterial(scene->GetComponent<View>(plane));
		ImGui::ShowDemoWindow();
		ImGui::Begin("test");
		auto light = scene->GetComponent<tEngine::Light>(Light);
		auto light_transform = scene->GetComponent<Transform>(Light);
		ImGui::DragFloat3("lightPos",&light_transform->getLocalPosition()[0],0.1,-10,10);
		ImGui::DragFloat("lightArea", &light->lightArea, 0.1, 0.01, 30);
		glm::vec3 eulerAngle = light_transform->getLocalEulerAngle();
		ImGui::DragFloat3("lightRotation", &eulerAngle[0],1,-360,360);
		light_transform->setOrientation(eulerAngle);
		ImGui::InputFloat3("lightScale", &light_transform->getLocalScale()[0]);
		ImGui::InputFloat4("area", &light->lightArea);
		ImGui::DragFloat3("uKd",&uKd[0],.01,0,3);
		ImGui::DragFloat3("uKs",&uKs[0],.01,0,15);
		ImGui::DragFloat("lightIntensity",&light->lightIntensity,.01,0,10);
		ImGui::End();
	

		});
	auto cam = scene->GetComponent<Camera>(camera);
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
		world.getRenderWorld().meshRenderer->Draw(debugQuat,cb,info);
		});
	context->Record([&](double timeDelta, CommandBufferHandle& cb) {
		//Collect Shadow map
		world.getRenderWorld().Render(cb, context->swapChain, context->getImageIdx());

		
		});

	
	auto threadContext = context->AddThreadContext();
	context->Loop(threadContext);

	return 0;
}