#pragma once
#include<unordered_map>
#include"Asset.h"

//����ظ�����ͬһ����Դ�����⣬���е�Ӳ���ϵ���Դ��ͨ������ṹ����
//������Ҫ֪��Ӳ���ϵ���Դ��Ӧ������һ����Դ
//�µ����⣺����ļ����س�һ����Դ���Ȱ�һ����������key
namespace tEngine {
	class ResourceLoadManager {
		
		using Key = std::string;
		std::string BaseFolder;// = STR2(ASSET_PATH);
		std::unordered_map<Key, std::weak_ptr<Asset>>assetPool;
		std::shared_ptr<Asset> GetResource(Key name) {
			if (assetPool.count(name) == 0)return nullptr;
			return std::move(assetPool[name].lock());
		}
		Key getKey(std::string name) {
			return name;

			const char* pre = "Assets/";
			auto preStr = std::strstr(name.c_str(), pre);
			size_t idx_begin = preStr - name.c_str();
			idx_begin += std::strlen(pre);
			auto result = name.substr(idx_begin);
			return result;
		}
		
	public:
		ResourceLoadManager();
		friend std::shared_ptr<MeshAsset> LoadMesh(std::string filename);
		friend std::shared_ptr<ImageAsset> LoadImage(std::string filename);
		friend std::shared_ptr<ShaderAsset> LoadShader(std::string filename);
		static ResourceLoadManager manager;
	};
	//std::shared_ptr<MeshAsset> LoadMesh(std::string filename);
}