#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"GameObject.h"
#include"World.h"
#include"renderer.h"
#include"Material.h"
#include"collide.h"
using namespace tEngine;
using namespace tPhysics;
Mesh lines;
Vertex V(glm::vec3 position) {
	Vertex a;
	a.Position = position;
	a.Color = glm::vec4(1, 0, 0,1);
	return a;
}
void fill(Mesh& lines, glm::vec3 center, glm::vec3 half) {
	glm::vec3 half_x = half; half_x.x = -half_x.x;
	glm::vec3 half_y = half; half_y.y = -half_y.y;
	glm::vec3 half_z = half; half_z.z = -half_z.z;
	glm::vec3 half_yz = half_y; half_yz.z = -half_yz.z;
	glm::vec3 half_xy = half_x; half_xy.y = -half_xy.y;
	glm::vec3 half_zx = half_x; half_zx.z = -half_zx.z;
	glm::vec3 half_xyz = -half;

	lines.AppendLine(V(center + half), V(center + half_x));
	lines.AppendLine(V(center + half), V(center + half_y));
	lines.AppendLine(V(center + half), V(center + half_z));
	lines.AppendLine(V(center + half_x), V(center + half_xy));
	lines.AppendLine(V(center + half_x), V(center + half_zx));
	lines.AppendLine(V(center + half_y), V(center + half_yz));
	lines.AppendLine(V(center + half_y), V(center + half_xy));
	lines.AppendLine(V(center + half_z), V(center + half_zx));
	lines.AppendLine(V(center + half_z), V(center + half_yz));
	lines.AppendLine(V(center + half_xyz), V(center + half_xy));
	lines.AppendLine(V(center + half_xyz), V(center + half_zx));
	lines.AppendLine(V(center + half_xyz), V(center + half_yz));
}
int main() {
	ContextInit();
	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device; 
	tWorld world(device.get());
	auto meshAsset = tEngine::LoadMesh("Sphere.obj");
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image = createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	GameObject character = GameObject_::Create();
	GameObject visAABB = GameObject_::Create();
	visAABB->AddComponent<MeshRenderer>(std::make_shared<Material>(tShaderInterface::requestVertexColorShader(device.get())))->material->graphicsState.topology=vk::PrimitiveTopology::eLineList;
	visAABB->AddComponent<MeshBuffer>();
	character->AddComponent<MeshRenderer>(std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get())))->material->SetImage("_MainTex", image);;
	character->AddComponent<MeshBuffer>()->setMeshUpload(meshAsset->mesh,device.get());
	//character->AddComponent<MeshBuffer>()->setMeshUpload(Mesh::UnitBox(),device.get());


	character->AddComponent<SphereCollider>();
	auto camera = Camera::Create();
	camera->getComponent<Camera>()->transform.m_windowSize = glm::uvec2(context.swapChain->getExtent().width, context.swapChain->getExtent().height);
	camera->getComponent<Camera>()->transform.update();
	
	world.AddGameObject(camera);
	world.AddGameObject(character);
	world.AddGameObject(visAABB);
	CameraSystem cam_sys;
	cam_sys.setCamera(&camera->getComponent<Camera>()->transform);
	
	//lines.AppendLine(V(glm::vec3(-10, 0, 0)), V(glm::vec3(10, 0, 0)));
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
	context.Update([&](double timeDelta) {
		position = character->transform.getPosition();
		rotation = character->transform.getEulerAngle();
		scale = character->transform.getScale();
		ImGui::Begin("Transform");
		ImGui::InputFloat3("position", &position[0]); character->transform.setPosition(position);
		ImGui::InputFloat3("rotation", &rotation[0]); character->transform.setOrientation(rotation);
		ImGui::InputFloat3("scale", &scale[0]); character->transform.setScale(scale);
		ImGui::End();
		auto& io = ImGui::GetIO();
		cam_sys.ExecuteAllComponents(timeDelta);
		lines.vertices.clear();
		lines.indices.clear();
		character->getComponent<SphereCollider>()->UpdateDerivedData();
		auto aabb = character->getComponent<SphereCollider>()->getAABB();
		auto center = aabb->getCenter();
		auto halfSizes = aabb->getHalfSize();
		fill(lines, aabb->getCenter(), aabb->getHalfSize());
		visAABB->getComponent<MeshBuffer>()->setMeshUpload(lines, device.get());
		//visAABB->getComponent<MeshBuffer>
		});
	context.Record([&](double timeDelta, CommandBufferHandle& cb) {
		world.getRenderWorld().Render(cb, context.swapChain, context.getImageIdx());
		});
	context.Loop(context.AddThreadContext());
	return 0;
}