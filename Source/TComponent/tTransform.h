#pragma once
#include"glm.hpp"
#include"glm/gtx/rotate_vector.hpp"
namespace tEngine {
	//Easy way to tranform from position,rotation and scale to matrix
	class Transform {
		Transform& operator=(const Transform& trans) {
			position = trans.position;
			rotation = trans.rotation;
			scale = trans.scale;
			return *this;
		}
	
	public:
		glm::vec3 position = glm::vec3(0, 0, 0);
		glm::vec3 rotation = glm::vec3(0, 0, 0);
		glm::vec3 scale = glm::vec3(1, 1, 1);
		const glm::mat4& Matrix() {
			matrix = glm::mat4(1);
			matrix=glm::scale(glm::mat4(1), scale)* matrix;
			matrix = glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(1, 0, 0)) * matrix;
			matrix = glm::rotate(glm::mat4(1), glm::radians(rotation.y), glm::vec3(0, 1, 0)) * matrix;
			matrix = glm::rotate(glm::mat4(1), glm::radians(rotation.z), glm::vec3(0, 0, 1)) * matrix;
			matrix= glm::translate(glm::mat4(1), position)* matrix;
			return matrix;
		}
	private:
		glm::mat4 matrix=glm::mat4(1);//data to gpu
	};
}