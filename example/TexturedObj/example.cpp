#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"World.h"
#include"renderer.h"
#include"Material.h"
#include"Component.h"
#include"RigidBody.h"
using namespace tEngine;
int main() {
	ContextInit();
	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device; 
	tWorld world(device.get());
	auto& scene = world.getEcsManager();
	auto camera = createCamera(&scene, glm::uvec2(context.swapChain->getExtent().width, context.swapChain->getExtent().height));
	world.getRenderWorld().SetCamera(camera);
	auto meshAsset = tEngine::LoadMesh("Obj/Marry.obj");
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image = createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	auto character = createGameObject(&scene);
	auto material = std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get()));;
	material->SetImage("_MainTex",image);
	auto meshBuffer =  std::make_shared<MeshBuffer>();
	meshBuffer->setMesh(meshAsset->mesh);
	meshBuffer->Upload(device.get());
	scene.GetComponent<View>(character)->material = material;
	scene.GetComponent<MeshFilter>(character)->setMeshBuffer(meshBuffer);
	CameraSystem cam_sys;
	cam_sys.ecsManager = &scene;
	cam_sys.setCamera(camera);
	context.Update([&](double timeDelta) {
		auto& io = ImGui::GetIO();
		cam_sys.ExecuteAllComponents(timeDelta);
		});
	context.Record([&](double timeDelta, CommandBufferHandle& cb) {
		world.getRenderWorld().Render(cb, context.swapChain, context.getImageIdx());
		});
	context.Loop(context.AddThreadContext());
//	scene.DestroyEntity(camera);
	//scene.DestroyEntity(character);
	return 0;
}