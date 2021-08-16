#include"EngineContext.h"
#include"Camera.h"
#include"ShaderVariable.h"
#include"World.h"
#include"renderer.h"
#include"Material.h"
#include"collide.h"
#include"RigidBody.h"
#include"Light.h"
using namespace tEngine;
using namespace tEngine;

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

#define CREATEOBJ(NAME,ecsManager) EntityID NAME = createGameObject(ecsManager);ecsManager->entity_map[NAME]=#NAME;
#define CREATE(NAME,ecsManager)  NAME = createGameObject(ecsManager);ecsManager->entity_map[NAME]=#NAME;
int main() {
	ContextInit();
	auto& context = tEngine::tEngineContext::context;
	auto& device = context.device; 
	
	tWorld world(device.get());
	auto scene = world.getEcsManager();
	auto camera = createCamera(scene, glm::uvec2(context.swapChain->getExtent().width, context.swapChain->getExtent().height));
	world.getRenderWorld().SetCamera(camera);
	auto meshAsset = tEngine::LoadMesh("Sphere.obj");
	auto MariAsset = tEngine::LoadMesh("Obj/Marry.obj");
	auto imageAsset = tEngine::LoadImage("Obj/MC003_Kozakura_Mari.png");
	auto image = createImage(device.get(),
		ImageCreateInfo::immutable_2d_image(imageAsset->width, imageAsset->height, VK_FORMAT_R8G8B8A8_UNORM), imageAsset, nullptr);
	EntityID cards[16];
	auto meshBuffer = std::make_shared<MeshBuffer>();
	meshBuffer->setMesh(Mesh::UnitBox());
	meshBuffer->Upload(device.get());
	

	CREATEOBJ(Light, scene)
	scene->AddComponent<tEngine::Light>(Light, device.get());
	scene->GetComponent<View>(Light)->interactShadow(false);
	scene->GetComponent<View>(Light)->material = std::make_shared<Material>(tShaderInterface::requestTexturedShader(device.get()));
	scene->GetComponent<MeshFilter>(Light)->setMeshUpload(Mesh::UnitBox(), device.get());
	scene->GetComponent<Transform>(Light)->setPosition(glm::vec3(0, 6, 10));
	scene->GetComponent<Transform>(Light)->setOrientation(glm::vec3(-120, 50, 0));
	scene->GetComponent<Transform>(Light)->setScale(glm::vec3(1, 1, 1));


//	CREATEOBJ(character,scene)
	CREATEOBJ(plane,scene)
	
	auto shadowShader = tShader::Create(device.get());
	shadowShader->SetShaderModule({ "draw.vsh","PCSS.fsh" }, { vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment });
	auto material = std::make_shared<Material>(shadowShader.get());
	material->SetImage("_MainTex", image);
	material->SetValue("uKd", glm::vec3(0.7, 0.7, 0.7));
	material->SetValue("uKs",glm::vec3(0.3,0.3,0.3));
	//scene->GetComponent<View>(character)->material = material;
//	scene->GetComponent<View>(character)->interactShadow(true);
	scene->GetComponent<View>(plane)->material = material;
	scene->GetComponent<View>(plane)->interactShadow(true);
	scene->GetComponent<MeshFilter>(plane)->setMeshUpload(Mesh::UnitBox(),device.get());
	scene->GetComponent<Transform>(plane)->setOrientation(glm::vec3(0, 0, 0));
	scene->GetComponent<Transform>(plane)->setScale(10,0.01,10);
	scene->AddComponent<PlaneCollider>(plane);
	scene->AddComponent<RigidBody>(plane);
	scene->GetComponent<RigidBody>(plane)->setInverseMass(0);
//	scene->GetComponent<MeshFilter>(character)->setMeshUpload(MariAsset->mesh,device.get());// = material;
	for (unsigned i = 0; i < 3; ++i) {
		CREATE(cards[i], scene);
		scene->AddComponent<RigidBody>(cards[i])->setInertiaTensor(CuboidInertiaTensor(1, 0.1, 1, 0.5));;
		scene->AddComponent<BoxCollider>(cards[i],Vector3(0.1,1,0.5));
		scene->GetComponent<Transform>(cards[i])->translate(Vector3(i * 0.5, 3, 0));
		scene->GetComponent<Transform>(cards[i])->setScale(Vector3(0.1, 1, 0.5));
		scene->GetComponent<View>(cards[i])->material = material;
		scene->GetComponent<View>(cards[i])->interactShadow(true);// = material;
		scene->GetComponent<MeshFilter>(cards[i])->setMeshBuffer(meshBuffer);// = material;
	}
	
	
	CREATEOBJ(visAABB, scene);
	scene->GetComponent<View>(visAABB)->material = std::make_shared<Material>(tShaderInterface::requestVertexColorShader(device.get()));
	scene->GetComponent<View>(visAABB)->material->graphicsState.topology = vk::PrimitiveTopology::eLineList;
	
	//scene->GetComponent<Transform>(cards[0])->translate(glm::vec3(0, 10, 0));


	CameraSystem cam_sys;
	cam_sys.ecsManager = scene;
	cam_sys.setCamera(camera);
//	auto character_transform = scene->GetComponent<Transform>(character);
	//lines.AppendLine(V(glm::vec3(-10, 0, 0)), V(glm::vec3(10, 0, 0)));
	Gravity gravity(Vector3(0,-10,0));
	world.getPhysicsWorld().registerForce(&gravity);
	context.timeRatio = 0;
	
	context.Update([&](double deltaTime) {
		{
			ImGui::Begin("Transform");
			glm::vec3 position;
			glm::vec3 rotation;
			glm::vec3 scale;
//			position = character_transform->getPosition();
//			rotation = character_transform->getLocalEulerAngle();
//			scale = character_transform->getScale();
//			ImGui::InputFloat3("position", &position[0]); character_transform->setPosition(position);
//			ImGui::InputFloat3("rotation", &rotation[0]); character_transform->setOrientation(rotation);
//			ImGui::InputFloat3("scale", &scale[0]); character_transform->setScale(scale);
			if (ImGui::Button("AddForce")) {
				auto rigid = scene->GetComponent<RigidBody>(cards[0]);

				auto transform = scene->GetComponent<Transform>(cards[0]);
				//	rigid->setCanSleep(false);
				rigid->addForceAtLocalPoint(transform, Vector3(100, 0, 100), Vector3(0, 1, 0));
			}
			if (ImGui::Button("Start")) {
				context.timeRatio = 1;
			}
			ImGui::End();

		
			
			auto& io = ImGui::GetIO();
			cam_sys.ExecuteAllComponents(deltaTime);
			lines.vertices.clear();
			lines.indices.clear();
			//	character->getComponent<MeshCollider>()->UpdateDerivedData();
			world.update(deltaTime);
			auto collider = scene->GetComponent<BoxCollider>(cards[0]);
			AABB aabb;
			aabb.setCenter(Vector3(collider->fclData->aabb_center[0], collider->fclData->aabb_center[1], collider->fclData->aabb_center[2]));
			aabb.setSize(Vector3(collider->fclData->side[0], collider->fclData->side[1], collider->fclData->side[2]));
			auto center = aabb.getCenter();
			auto halfSizes = aabb.getHalfSize();
			//LOG(LogLevel::Information, halfSizes.r, halfSizes.g, halfSizes.b);
//			fill(lines, aabb.getCenter(), aabb.getHalfSize());
//			scene->GetComponent<MeshFilter>(visAABB)->setMeshUpload(lines, device.get());
		}
		});
	context.FixedUpdate([&](double deltaTime) {
		
		world.getPhysicsWorld().runPhysics(deltaTime);
		world.getPhysicsWorld().startFrame();
		},1.0/120);
	context.Record([&](double timeDelta, CommandBufferHandle& cb) {
		world.getRenderWorld().Render(cb, context.swapChain, context.getImageIdx());
		});
	context.Loop(context.AddThreadContext());
	return 0;
}