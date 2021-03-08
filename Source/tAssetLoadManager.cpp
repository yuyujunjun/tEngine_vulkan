#include"tAssetLoadManager.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#endif
#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif
#include"stb_image/stb_image.h"
#include"tiny_obj_loader.h"
namespace tEngine {
	ResourceLoadManager ResourceLoadManager::manager;
	std::shared_ptr<MeshAsset> LoadMesh(std::string filename) {
		auto& manager = ResourceLoadManager::manager;
		auto key = manager.getKey(filename);
		auto res = manager.GetResource(key);

		if (res == nullptr) {
			std::string file = "../Assets/" + manager.BaseFolder + filename;
			//Mesh mesh;
			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string warn, err;
			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.c_str())) {
				throw std::runtime_error(warn + err);
			}
			Mesh mesh;
			std::unordered_map<Vertex, uint32_t> uniqueVertices{};
			for (const auto& shape : shapes) {
				for (const auto& index : shape.mesh.indices) {

					Vertex vertex;
					vertex.Position = { attrib.vertices[3 * index.vertex_index + 0],
										attrib.vertices[3 * index.vertex_index + 1],
										attrib.vertices[3 * index.vertex_index + 2] };
					if (index.texcoord_index != -1) {
						vertex.TexCoords = {
											 attrib.texcoords[2 * index.texcoord_index + 0],
											1.0 - attrib.texcoords[2 * index.texcoord_index + 1]
						};
					}
					if (index.normal_index != -1) {
						vertex.Normal = { attrib.normals[3 * index.normal_index + 0],
											attrib.normals[3 * index.normal_index + 1],
											attrib.normals[3 * index.normal_index + 2] };
					}
					vertex.Color = { 1.0f,1.0f,1.0f,1.0f };
					if (uniqueVertices.count(vertex) == 0) {
						uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
						mesh.vertices.push_back(vertex);
					}
					mesh.indices.push_back(uniqueVertices[vertex]);

				}
			}
			auto meshAsset = std::make_shared<MeshAsset>();
			meshAsset->mesh = mesh;
			res = meshAsset;
			manager.assetPool[key] = res;
			Log("Load Mesh", LogLevel::Performance);
		}
		return std::static_pointer_cast<tEngine::MeshAsset>(res);
	}
}