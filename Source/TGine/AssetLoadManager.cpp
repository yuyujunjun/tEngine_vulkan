#include"AssetLoadManager.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#endif
#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#endif
#define STR1(R)  #R
#define STR2(R)  STR1(R)
#include"stb_image/stb_image.h"
#include"tiny_obj_loader.h"
#include"Log.h"
namespace tEngine {
	ResourceLoadManager ResourceLoadManager::manager;
	ResourceLoadManager::ResourceLoadManager() {
		BaseFolder = STR2(ASSET_PATH) + std::string("/Assets/");
	}
	std::vector<uint32_t> read_binary_file(const std::string& filename, const uint32_t count)
	{
		std::vector<uint32_t> data;

		std::ifstream file;

		file.open(filename, std::ios::in | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file: " + filename);
		}

		uint64_t read_count = count;
		if (count == 0)
		{
			file.seekg(0, std::ios::end);
			read_count = static_cast<uint64_t>(file.tellg());
			file.seekg(0, std::ios::beg);
		}

		data.resize(static_cast<size_t>(read_count/sizeof(uint32_t)));
		file.read(reinterpret_cast<char*>(data.data()), read_count);
		file.close();

		return data;
	}
	std::string read_text_file(const std::string& filename)
	{
		std::vector<std::string> data;

		std::ifstream file;

		file.open(filename, std::ios::in);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file: " + filename);
		}

		return std::string{ (std::istreambuf_iterator<char>(file)),
						   (std::istreambuf_iterator<char>()) };
	}
	std::shared_ptr<MeshAsset> LoadMesh(std::string filename) {
		auto& manager = ResourceLoadManager::manager;
		auto key = manager.getKey(filename);
		auto res = manager.GetResource(key);

		if (res == nullptr) {
			
			std::string file =  manager.BaseFolder + filename;
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
			LOGD( LogLevel::Performance, "Load Mesh");
		}
		return std::static_pointer_cast<tEngine::MeshAsset>(res);
	}
	std::shared_ptr<ImageAsset> LoadImage(std::string name) {
		auto& m = ResourceLoadManager::manager;
		auto image = std::static_pointer_cast<ImageAsset>(m.GetResource(m.getKey(name)));
		//Load image
		if (image == nullptr) {
			LOGD(LogLevel::Performance, "LoadImage " + m.getKey(name));
			
			int width, height, channels;
			stbi_set_flip_vertically_on_load(false);
			stbi_uc* pixels = stbi_load((m.BaseFolder + name).c_str(), &width, &height, &channels, STBI_rgb_alpha);
			image = std::make_shared<ImageAsset>();
			image->pixels = pixels;
			image->width = width;
			image->height = height;
			image->depth = 1;
			image->format = vk::Format::eR8G8B8A8Srgb;
			image->autoGenerateMips = false;
			image->enableRandomWrite = false;
			image->channels = 4;
			m.assetPool[m.getKey(name)] = image;
			
		}
		return image;
	}
	std::shared_ptr<ShaderAsset> LoadShader(std::string filename) {
		auto& m = ResourceLoadManager::manager;
		auto shader = std::static_pointer_cast<ShaderAsset>(m.GetResource(m.getKey(filename)));
		//Load image
		if (shader == nullptr) {
			std::string shaderFile = m.BaseFolder+ "Shader/" + filename+".spv";
			std::string jsonFile = m.BaseFolder + "Json/" + filename+".refl";
			LOGD(LogLevel::Performance,("LoadShader: " + m.getKey(filename)).c_str());
			auto shader_source= read_binary_file(shaderFile,0);
			auto json_code = read_text_file(jsonFile);
			shader = std::make_shared<ShaderAsset>();
			shader->shaderReflection = json_code;
			shader->shaderSource = shader_source;
			m.assetPool[m.getKey(filename)] = shader;

		}
		return shader;
	}
}