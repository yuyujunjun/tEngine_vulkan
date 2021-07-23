#pragma once
#include"glm.hpp"
#include"glm/gtx/rotate_vector.hpp"
#include"glm/gtx/quaternion.hpp"
#include<glm/gtc/matrix_transform.hpp>
namespace tEngine {
	struct Geo;
	class Transform {
		glm::vec3 position;
		glm::quat orientation;
		glm::vec3 scale;
		glm::mat4 matrix;
		glm::mat4 worldMatrix;
		Transform* parent=0;
	public:
		Transform() :position(0, 0, 0), orientation(1, 0, 0, 0), scale(1, 1, 1) {
			updateMtx();
		}
		Transform& operator=(const Transform& trans) {
			position = trans.position;
			orientation = trans.orientation;
			scale = trans.scale;
			matrix = trans.matrix;
			return *this;
		}
		inline void setOrientation(float x, float y, float z) {
			
			orientation=glm::quat(glm::radians(glm::vec3(x, y, z)));
		}
		inline void setOrientation(const glm::vec3& xyz) {
			orientation = glm::quat(glm::radians(xyz));
		}
		inline void setPosition(float x, float y, float z) {
			this->position = (glm::vec3(x, y, z));
		}
		inline void setPosition(const glm::vec3& position) {
			this->position = position;
		}
		inline void translate(const glm::vec3& offset) {
			this->position += offset;
		}
		inline void rotate(const glm::quat& rotation) {
			this->orientation = rotation * this->orientation;
		}
		inline void rotate(const glm::vec3& axis, const float value) {
			glm::quat q(0, axis * value);
			q *= this->orientation;
			this->orientation += q * 0.5f;
		}
		inline void setScale(float x, float y, float z) {
			this->scale = glm::vec3(x, y, z);
		}
		inline void setScale(const glm::vec3& xyz) {
			this->scale = xyz;
		}
		inline void setParent(Transform* parent) {
			this->parent = parent;
		}
		const glm::vec3& getPosition()const {
			return position;
		}
		glm::vec3& getPosition() {
			return position;
		}
		const glm::vec3& getScale()const {
			return scale;
		}
		glm::vec3& getScale() {
			return scale;
		}
		const glm::quat& getOrientation() const {
			return orientation;
		}
		const glm::vec3& getEulerAngle()const {
			return glm::degrees(glm::eulerAngles(orientation));
		}
		const glm::mat3& getOrientationMat()const {
			return glm::toMat3(orientation);
		}
		const glm::mat4& getMtx()const {
			//return updateMtx();
			return worldMatrix;
		}
		inline const glm::mat4&  updateMtx() {
			 matrix=glm::translate(glm::mat4(1),position)*glm::toMat4(orientation)*glm::scale(glm::mat4(1),scale);
			 if (parent) {
				 parent->updateMtx();
			 }
			 if (parent) {
				 worldMatrix = parent->getMtx()*matrix;
			 }
			 else {
				 worldMatrix = matrix;
			 }
			 return worldMatrix;
		}
		inline glm::vec3 transformDirection(const glm::vec3& d)const {
			return glm::vec3(
			worldMatrix[0][0]*d[0]+worldMatrix[1][0]*d[1]+worldMatrix[2][0]*d[2],
			worldMatrix[0][1]*d[0]+worldMatrix[1][1]*d[1]+worldMatrix[2][1]*d[2],
			worldMatrix[0][2]*d[0]+worldMatrix[1][2]*d[1]+worldMatrix[2][2]*d[2]
			);
		}
		inline glm::vec3 transformInverseDirection(const glm::vec3& d)const {
			return glm::vec3(
				worldMatrix[0][0] * d[0] + worldMatrix[0][1] * d[1] + worldMatrix[0][2] * d[2],
				worldMatrix[1][0] * d[0] + worldMatrix[1][1] * d[1] + worldMatrix[1][2] * d[2],
				worldMatrix[2][0] * d[0] + worldMatrix[2][1] * d[1] + worldMatrix[2][2] * d[2]
			);
		}
	};

	//Easy way to tranform from position,rotation and scale to matrix
	//class Transform {
	//	Transform& operator=(const Transform& trans) {
	//		position = trans.position;
	//		rotation = trans.rotation;
	//		scale = trans.scale;
	//		return *this;
	//	}
	//
	//public:
	//	glm::vec3 position = glm::vec3(0, 0, 0);
	//	glm::vec3 rotation = glm::vec3(0, 0, 0);
	//	glm::vec3 scale = glm::vec3(1, 1, 1);
	//	const glm::mat4& Matrix() {
	//		matrix = glm::mat4(1);
	//		matrix=glm::scale(glm::mat4(1), scale)* matrix;
	//		matrix = glm::rotate(glm::mat4(1), glm::radians(rotation.x), glm::vec3(1, 0, 0)) * matrix;
	//		matrix = glm::rotate(glm::mat4(1), glm::radians(rotation.y), glm::vec3(0, 1, 0)) * matrix;
	//		matrix = glm::rotate(glm::mat4(1), glm::radians(rotation.z), glm::vec3(0, 0, 1)) * matrix;
	//		
	//		matrix= glm::translate(glm::mat4(1), position)* matrix;
	//		return matrix;
	//	}
	//private:
	//	glm::mat4 matrix=glm::mat4(1);//data to gpu
	//};
}