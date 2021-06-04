#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"TextureFormatLayout.h"
#include"bufferHelper.h"
#include<random>
#include"Log.h"
#include"GraphicsState.h"
#include<glm/gtx/quaternion.hpp>
#include"ComponentFactory.h"
using namespace tEngine;

struct requireRender {};
ComponentManager<requireCamera> cameraList;
ComponentManager<requireRender> requireRenderList;


int main() {

	
	ContextInit();
	CameraComponent c[2];
	auto camera_id = 0;
	CameraSystem cam;
	cam.setCamera(c);
	cam.setWindowSize(glm::u32vec2(tEngineContext::context.swapChain->getExtent().width, tEngineContext::context.swapChain->getExtent().height));
	auto context = &tEngineContext::context;
	auto& device = context->device;
	

	auto shadowPass = getCollectShadowPass(device.get(), vk::Format::eR32Sfloat);
	auto shadowMapCreateInfo = ImageCreateInfo::render_target(1024, 1024, (VkFormat)vk::Format::eR32Sfloat);
	shadowMapCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	auto ShadowMap = createImage(device.get(), shadowMapCreateInfo);

	auto renderPass = getSingleRenderpass(device.get(), context->swapChain->getFormat());
	
	GameObject_ character;// = std::make_shared<GameObject_>();
	GameObject_ plane;// = std::make_shared<GameObject_>();
	GameObject_ Light;// = std::make_shared<GameObject_>();
	
	///Shader
	
	//ShadowMap Shader
	auto shadowShader = tShader::Create(device.get());
	shadowShader->SetShaderModule({ "ShadowPass.vsh","ShadowPass.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto shadowMateial=shadowShader->getInterface();
	//BlingPhong shader
	auto shader = tShader::Create(device.get());
	shader->SetShaderModule({ "draw.vsh","Pcss.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto CameraBuffer = shader->requestBufferRange("CameraMatrix");
	character.setMaterial(shader->getInterface());
	plane.setMaterial(shader->getInterface());
	//Normal shader
	Light.setMaterial(tShaderInterface::requestTexturedShader(device.get()));
	//Post Shader
	auto postShader = tShader::Create(device.get());
	postShader->SetShaderModule({ "Quad.vsh","MainTex.fsh" }, { vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment });
	auto debugQuad = postShader->getInterface();


	
	//Mesh
	auto meshAsset = tEngine::LoadMesh("Obj/Marry.obj");
	character.setMesh(meshAsset->mesh);
	plane.setMesh(Mesh::UnitSquare());
	plane.transform.rotation = glm::vec3(-90,0,0);
	plane.transform.scale = glm::vec3(10, 10, 10);
	{
		auto box = Mesh::UnitBox();
		Transform transform;
		transform.scale = glm::vec3(0.1, 0.1, 0.2);
		FlipFace(box);
		for (auto& b : box.vertices) {
			b.Position = transform.Matrix() * glm::vec4(b.Position.x, b.Position.y, b.Position.z, 1);
		}
		Light.setMesh(box);
	}
	//Light->transform.scale = glm::vec3(.1,.1,.1);
	Light.transform.position = glm::vec3(1, 2, 2);
	Light.transform.rotation = glm::vec3(-120,0,0);
	Light.transform.scale = glm::vec3(1, 1, 1);
	float area = 1;
	auto uploadTransform = [&](GameObject_& obj) {
		obj.material->SetValue(ShaderString(SV::_MATRIX_M), obj.transform.Matrix());
	};
	float lightIntensity = .2;
	glm::vec3 uKd = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 uKs = glm::vec3(0.2, 0.2, 0.2);
	
	

	//Load Textue
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image =createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	
	//Config shaderInterface
	character.material->SetImage("_MainTex", image);
	plane.material->SetImage("_MainTex", tImage::requestWhiteImage(device.get()));

	auto lightArea = glm::vec4(-15, 15, -15, 15);
	GraphicsState debugState;
	GraphicsState shadowMapState;
	shadowMapState.depthBias.depthBiasEnable = true;
	shadowMapState.depthBias.depthBiasConstantFactor = 1e5;
	shadowMapState.depthBias.depthBiasSlopeFactor = 1;
	shadowMapState.depthBias.depthBiasClamp = 0.2;
	auto uploadMaterial = [&](GameObject_& obj) {
		obj.material->SetImage("_ShadowMap", ShadowMap, ShadowMap->get_view()->get_float_view(), tEngine::StockSampler::LinearShadow);
		obj.material->SetValue("uKd", uKd);
		obj.material->SetValue("uKs", uKs);
		obj.material->SetValue("lightPosArea", glm::vec4(Light.transform.position,area));
		obj.material->SetValue<float>("lightIntensity", lightIntensity);
		obj.material->SetValue("cameraPos", cam.getCameraPosition());
		obj.material->SetValue("world_to_shadow", Ortho(lightArea.x, lightArea.y, lightArea.z, lightArea.w, 40)* glm::lookAt(Light.transform.position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
		obj.material->SetValue("halfblockSize", 8);
		obj.material->SetValue("maxKernelSize",8);
		obj.material->SetValue("depthMapSize",glm::vec2(ShadowMap->get_width(),ShadowMap->get_height()) );
	};

	std::vector<GameObject_> needRender;//= { character,plane };
	needRender.emplace_back(character);
	needRender.emplace_back(plane);
	std::vector<std::shared_ptr<Material>> needCamera = { character.material ,plane.material,Light.material };
	auto updateCamera = [&]() {
		CameraBuffer->NextRangenoLock();
		for (auto& shader : needCamera) {
			shader->SetBuffer("CameraMatrix", CameraBuffer->buffer(), CameraBuffer->getOffset());
		}
		uploadCameraMatrix(cam.getMatrix(), Perspective(context->swapChain->getExtent()), character.material->shader.get());

	};
	context->Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		if (ImGui::IsKeyPressed(GLFW_KEY_T,0)) {
			camera_id += 1;
			camera_id = camera_id % 2;
			cam.setCamera(c + camera_id);
		}
		updateCameraBehavior(io, cam);
		shadowPass->SetImageView("shadowMap", ShadowMap, ShadowMap->get_view()->getDefaultView());
		shadowPass->setTransientImageView("depth");
		//renderPass->SetImageView("shadow", ShadowMap, ShadowMap->get_view()->get_float_view());
		renderPass->SetImageView("back", context->swapChain->getImage(context->getImageIdx()));
		renderPass->setTransientImageView("depth");
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		ImGui::ShowDemoWindow();
		
		ImGui::Begin("test");
		ImGui::DragFloat3("lightPos",&Light.transform.position[0],0.1,-10,10);
		ImGui::DragFloat("lightArea", &area, 0.1, 0.01, 30);
		ImGui::DragFloat3("lightRotation", &Light.transform.rotation[0],1,-360,360);
		ImGui::InputFloat3("lightScale", &Light.transform.scale[0]);
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
			auto view = (glm::lookAt(Light.transform.position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
			auto projection = Ortho(lightArea.x, lightArea.y, lightArea.z, lightArea.w, 40);
			CameraBuffer->NextRangenoLock();
			shadowMateial->SetBuffer("CameraMatrix", CameraBuffer->buffer(), CameraBuffer->getOffset());
			shadowMateial->SetValueOnBuffer(ShaderString(SV::_MATRIX_V), view);
			shadowMateial->SetValueOnBuffer(ShaderString(SV::_MATRIX_P), projection);
			shadowMateial->SetValueOnBuffer(ShaderString(SV::_MATRIX_VP), projection* view);
		//	shadowMateial->SetValueOnBuffer(ShaderString(SV::_INV_MATRIX_VP), glm::inverse(projection* view));
			
			
			for (auto& r : needRender) {
				auto bufferRange = shadowMateial->getShader()->requestBufferRange("ModelMatrix");
				bufferRange->NextRangenoLock();
				shadowMateial->SetBuffer("ModelMatrix",bufferRange->buffer(),bufferRange->getOffset());
				shadowMateial->SetValueOnBuffer(ShaderString(SV::_MATRIX_M), r.transform.Matrix());
				flushGraphicsShaderState(shadowMateial.get(),shadowMapState, cb, shadowPass.get(), 0);
				DrawMesh(r.meshbuffer.get(), cb);
			}
			cb->endRenderPass();
			
			auto& frameBuffer = renderPass->requestFrameBuffer();
			
			auto viewPort = frameBuffer->getViewPort();
			cb->beginRenderPass(renderPass, frameBuffer, true);
			cb->setViewport(frameBuffer->getViewPort());
			cb->setScissor(0,frameBuffer->getRenderArea());
			updateCamera();
			for (auto& r : needRender) {
				uploadTransform(r);
				uploadMaterial(r);
				r.material->flushBuffer();
				flushGraphicsShaderState(r.material->shader.get(),r.material->graphicsState, cb, renderPass.get(), 0);
				DrawMesh(r.meshbuffer.get(), cb);
			}
			uploadTransform(Light);
			Light.material->flushBuffer();
			flushGraphicsShaderState(Light.material->shader.get(),Light.material->graphicsState, cb, renderPass.get(), 0);
			DrawMesh(Light.meshbuffer.get(), cb);
			debugQuad->SetImage("_MainTex",ShadowMap,ShadowMap->get_view()->get_float_view(),tEngine::StockSampler::NearestShadow);
			flushGraphicsShaderState(debugQuad.get(),debugState, cb, renderPass.get(), 0);
			
			viewPort.height /= 3;
			viewPort.width /= 3;
		
			auto area = frameBuffer->getRenderArea();
			area.extent.height /= 3;
			area.extent.width /= 3;

			cb->setViewport(viewPort);
			cb->setDepthBias(0.1,0.2,10);
			cb->setScissor(0, frameBuffer->getRenderArea());
			DrawMesh(plane.meshbuffer.get(),cb,1);
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