#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"TextureFormatLayout.h"
#include"bufferHelper.h"
#include<random>
using namespace tEngine;

int main() {


	ContextInit();

	auto context = &tEngineContext::context;
	auto& device = context->device;

	ThreadContext* threadContext = new ThreadContext(context);
	auto renderPass = getShadowMapPass(device.get(), context->swapChain->getFormat(), (vk::Format)context->swapChain->getDepth()->getFormat());
	GameObject character=std::make_shared<GameObject_>();
	GameObject plane = std::make_shared<GameObject_>();
	GameObject Light = std::make_shared<GameObject_>();
	
	///Shader
	//ShadowMap Shader
	auto shadowShader = tShader::Create(device.get());
	shadowShader->SetShaderModule({ "ShadowPass.vsh","ShadowPass.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto shadowMateial=shadowShader->getInterface();
	//BlingPhong shader
	auto shader = tShader::Create(device.get());
	shader->SetShaderModule({ "draw.vsh","Pcss.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto CameraBuffer = shader->requestBufferRange("CameraMatrix");
	character->setMaterial(  shader->getInterface());
	plane->setMaterial(shader->getInterface());
	//Normal shader
	auto lightShader = tShader::Create(device.get());
	lightShader->SetShaderModule({ "draw.vsh","draw.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	Light->setMaterial( lightShader->getInterface());
	//Post Shader
	auto postShader = tShader::Create(device.get());
	postShader->SetShaderModule({ "Quad.vsh","MainTex.fsh" }, { vk::ShaderStageFlagBits::eVertex,vk::ShaderStageFlagBits::eFragment });
	auto debugQuad = postShader->getInterface();
	std::vector<std::shared_ptr<Material>> needCamera = { character->material ,plane->material,Light->material };
	auto updateCamera=[&]() {
		CameraBuffer->NextRangenoLock();
		for (auto& shader : needCamera) {
			shader->SetBuffer("CameraMatrix", CameraBuffer->buffer(), CameraBuffer->getOffset());
		}
		uploadCameraMatrix(tEngineContext::context.cameraManipulator.getMatrix(), Perspective(context->swapChain->getExtent()), character->material->shader.get(),CameraBuffer->buffer());
		
	};
	std::vector<GameObject> needRender = {character,plane};
	//Mesh
	auto meshAsset = tEngine::LoadMesh("Obj/Marry.obj");
	character->setMesh(meshAsset->mesh);
	plane->setMesh(Mesh::UnitSquare());
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
		Light->setMesh(box);
	}
	//Light->transform.scale = glm::vec3(.1,.1,.1);
	Light->transform.position = glm::vec3(0, 5, -2);
	Light->transform.rotation = glm::vec3(-120,0,0);
	Light->transform.scale = glm::vec3(1, 1, 1);
	auto uploadTransform = [&](GameObject& obj) {
		obj->material->SetValue(ShaderString(SV::_MATRIX_M), obj->transform.Matrix());
	};
	float lightIntensity = 2;
	glm::vec3 uKd = glm::vec3(0.2, 0.2, 0.2);
	glm::vec3 uKs = glm::vec3(0.2, 0.2, 0.2);


	//Load Textue
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image =createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);

	//Config shaderInterface
	character->material->SetImage("_MainTex", image);
	plane->material->SetImage("_MainTex", tImage::requestWhiteImage(device.get()));
	auto shadowMapCreateInfo = ImageCreateInfo::render_target(context->swapChain->getExtent().width, context->swapChain->getExtent().height, (VkFormat)vk::Format::eD32Sfloat);
	shadowMapCreateInfo.usage |= (VkImageUsageFlagBits)vk::ImageUsageFlagBits::eInputAttachment;
	shadowMapCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	auto ShadowMap = createImage(device.get(), shadowMapCreateInfo);
	auto lightArea = glm::vec4(-10, 10, -10, 10);
	auto uploadMaterial = [&](GameObject& obj) {
		obj->material->SetImage("_ShadowMap", ShadowMap, ShadowMap->get_view()->get_float_view(), tEngine::StockSampler::NearestShadow);
		obj->material->SetValue("uKd", uKd);
		obj->material->SetValue("uKs", uKs);
		obj->material->SetValue<float>("lightIntensity", lightIntensity);
		obj->material->SetValue("lightPos", Light->transform.position);
		obj->material->SetValue("cameraPos", tEngineContext::context.cameraManipulator.getCameraPosition());
		obj->material->SetValue("world_to_light", Ortho(lightArea.x,lightArea.y,lightArea.z,lightArea.w,20)* glm::inverse(Light->transform.Matrix()));
	
	};
	context->Update([&](double timeDelta) {
		renderPass->SetImageView("shadow", ShadowMap);
		renderPass->SetImageView("back", context->swapChain->getImage(context->imageIdx));
		renderPass->SetImageView("depth", context->swapChain->getDepth());
		renderPass->setClearValue("back", { 0,0,0,1 });
		renderPass->setDepthStencilValue("depth", 1);
		
		ImGui::Begin("test");
		ImGui::DragFloat3("lightPos",&Light->transform.position[0],0.1,-10,10);
		ImGui::DragFloat3("lightRotation", &Light->transform.rotation[0],1,-360,360);
		ImGui::InputFloat3("lightScale", &Light->transform.scale[0]);
		ImGui::InputFloat4("area", &lightArea[0]);
		ImGui::DragFloat3("uKd",&uKd[0],.01,0,3);
		ImGui::DragFloat3("uKs",&uKs[0],.01,0,15);
		ImGui::DragFloat("lightIntensity",&lightIntensity,.01,0,10);
		ImGui::End();

		});
		context->Record([&](double timeDelta, CommandBufferHandle& cb) {
			auto& frameBuffer = renderPass->requestFrameBuffer();
			auto viewPort = frameBuffer->getViewPort();
			cb->beginRenderPass(renderPass, frameBuffer, true);
			cb->setViewport(viewPort);
			cb->setScissor(0, frameBuffer->getRenderArea());
			//ÉèÖÃshadowInterface
			auto view = glm::inverse(Light->transform.Matrix());
			auto projection = Ortho(lightArea.x, lightArea.y, lightArea.z, lightArea.w, 20);
			CameraBuffer->NextRangenoLock();
			shadowMateial->SetBuffer("CameraMatrix", CameraBuffer->buffer(), CameraBuffer->getOffset());
			shadowMateial->SetValue(ShaderString(SV::_MATRIX_V), view);
			shadowMateial->SetValue(ShaderString(SV::_MATRIX_P), projection);
			shadowMateial->SetValue(ShaderString(SV::_MATRIX_VP), projection* view);
			shadowMateial->SetValue(ShaderString(SV::_INV_MATRIX_VP), glm::inverse(projection* view));
			
			
			for (auto& r : needRender) {
				auto bufferRange = shadowMateial->getShader()->requestBufferRange("ModelMatrix");
				bufferRange->NextRangenoLock();
				shadowMateial->SetBuffer("ModelMatrix",bufferRange->buffer(),bufferRange->getOffset());
				shadowMateial->SetValue(ShaderString(SV::_MATRIX_M), r->transform.Matrix());
				flushGraphicsShaderState(shadowMateial.get(), cb, renderPass.get(), 0);
				DrawMesh(r->meshbuffer.get(), cb);
			}
			cb->NextSubpass(vk::SubpassContents::eInline);
			updateCamera();
			for (auto& r : needRender) {
				uploadTransform(r);
				uploadMaterial(r);
				r->material->flushMaterialState();
				flushGraphicsShaderState(r->material->shader.get(), cb, renderPass.get(), 1);
				DrawMesh(r->meshbuffer.get(), cb);
			}
			uploadTransform(Light);
			Light->material->flushMaterialState();
			flushGraphicsShaderState(Light->material->shader.get(), cb, renderPass.get(), 1);
			DrawMesh(Light->meshbuffer.get(), cb);
			debugQuad->SetImage("_MainTex",ShadowMap,ShadowMap->get_view()->get_float_view(),tEngine::StockSampler::NearestShadow);
			flushGraphicsShaderState(debugQuad.get(), cb, renderPass.get(), 1);
			
			viewPort.height /= 3;
			viewPort.width /= 3;
			auto area = frameBuffer->getRenderArea();
			area.extent.height /= 3;
			area.extent.width /= 3;

			cb->setViewport(viewPort);
			cb->setDepthBias(0.1,0.2,10);
			cb->setScissor(0, frameBuffer->getRenderArea());
			DrawMesh(plane->meshbuffer.get(),cb,1);
			cb->endRenderPass();
			setImageLayout(cb, ShadowMap, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthAttachmentOptimal);

		
		});

	

	context->Loop(threadContext);


	


	
	
	
	delete threadContext;
	shader.reset();
	
	image.reset();
	//CameraBuffer.removeBuffer();
	//modelMatrix.removeBuffer();
	renderPass.reset();

	//delete context;
	//shaderInterface.reset();
	//CreateImageViewWithImage();


}