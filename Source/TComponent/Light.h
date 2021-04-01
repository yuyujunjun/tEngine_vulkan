#pragma once
#include"tTransform.h"
#include"glm/glm.hpp"
#include"glm/matrix.hpp"
#include<memory>
namespace tEngine {
	class tImage;
	using ImageHandle = std::shared_ptr<tImage>;
	class Light
	{
	public:
		
		const glm::mat4& world_to_lightMatrix();
		float lightIntensity = 10.f;
		float area = 1;
		Transform  transform;
		ImageHandle shadowMap;
	};
}
