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
		glm::vec3 worldPosition;
		glm::quat worldOrientation;
		glm::vec3 worldScale;
		glm::mat4 worldMatrix;
		bool dirty = true;
		Transform* parent=0;
	public:
		Transform() :position(0, 0, 0), orientation(1, 0, 0, 0), scale(1, 1, 1),dirty(true) {
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
			dirty = true;
		}
		inline void setOrientation(const glm::vec3& xyz) {
			orientation = glm::quat(glm::radians(xyz));
			dirty = true;
		}
		inline void setPosition(float x, float y, float z) {
			this->position = (glm::vec3(x, y, z));
			dirty = true;
		}
		inline void setPosition(const glm::vec3& position) {
			this->position = position;
			dirty = true;
		}
		inline void translate(const glm::vec3& offset) {
			this->position += offset;
			dirty = true;
		}
		inline void rotate(const glm::quat& rotation) {
			this->orientation = rotation * this->orientation;
			this->orientation = glm::normalize(this->orientation);
			dirty = true;
		}
		inline void rotate(const glm::vec3& axis, const float value) {
			glm::quat q(0, axis * value);
			q *= this->orientation;
			this->orientation += q * 0.5f;
			this->orientation = glm::normalize(this->orientation);
			dirty = true;
		}
		inline void setScale(float x, float y, float z) {
			this->scale = glm::vec3(x, y, z);
			dirty = true;
		}
		inline void setScale(const glm::vec3& xyz) {
			this->scale = xyz;
			dirty = true;
		}
		inline void setParent(Transform* parent) {
			this->parent = parent;
		}
	
		const glm::vec3& getPosition()const {
			return worldPosition;
		}
		glm::vec3& getLocalPosition() {
			return position;
		}
		const glm::vec3& getScale()const {
			return worldScale;
		}
		glm::vec3& getLocalScale() {
			return scale;
		}
		const glm::quat& getOrientation() const {
			return worldOrientation;
		}
		const glm::vec3& getLocalEulerAngle()const {
			return glm::degrees(glm::eulerAngles(orientation));
		}
	
		const glm::mat4& getMtx()const {
			return worldMatrix;
		}
		inline const glm::mat4&  updateMtx() {
			if (dirty) {
			
				matrix = glm::translate(glm::mat4(1), position) * glm::toMat4(orientation) * glm::scale(glm::mat4(1), scale);
				dirty = false;
			}
			 if (parent) {
				 parent->updateMtx();
			 }
			 if (parent) {
				 worldMatrix = parent->getMtx()*matrix;
				 worldScale = parent->worldScale * scale;
				 worldOrientation = parent->worldOrientation * orientation;
				 worldPosition = parent->worldPosition + position;
			 }
			 else {
				 worldMatrix = matrix;
				 worldOrientation = orientation;
				 worldPosition = position;
				 worldScale = scale;
			 }
			 return worldMatrix;
		}
		inline glm::mat3 getMat3()const {
			return glm::mat3(worldMatrix[0], worldMatrix[1], worldMatrix[2]);
		}
		//(RS)^{-1}=(RSS^{-2})^T
		inline glm::mat3 getInverseMat3()const {
			glm::mat3 scale_rotation = getMat3();
			glm::vec3 worldScale2 = worldScale * worldScale;
			scale_rotation[0] /= worldScale2.x;
			scale_rotation[1] /= worldScale2.y;
			scale_rotation[2] /= worldScale2.z;
			return glm::transpose(scale_rotation);
		}
		//R=RSS^{-1}
		inline glm::mat3 getOrientationMat3()const {
			glm::mat3 scale_rotation = getMat3();
			scale_rotation[0] /= worldScale.x;
			scale_rotation[1] /= worldScale.y;
			scale_rotation[2] /= worldScale.z;
			return scale_rotation;
		}
		//RS*d
		inline glm::vec3 transformDirection(const glm::vec3& d)const {
			return glm::vec3(
			worldMatrix[0][0]*d[0]+worldMatrix[1][0]*d[1]+worldMatrix[2][0]*d[2],
			worldMatrix[0][1]*d[0]+worldMatrix[1][1]*d[1]+worldMatrix[2][1]*d[2],
			worldMatrix[0][2]*d[0]+worldMatrix[1][2]*d[1]+worldMatrix[2][2]*d[2]
			);
		}
		//(RS)^{-1}*d
		inline glm::vec3 transformInverseDirection(const glm::vec3& d)const {

			glm::vec3 scaledDir = glm::vec3(
				worldMatrix[0][0] * d[0] + worldMatrix[0][1] * d[1] + worldMatrix[0][2] * d[2],
				worldMatrix[1][0] * d[0] + worldMatrix[1][1] * d[1] + worldMatrix[1][2] * d[2],
				worldMatrix[2][0] * d[0] + worldMatrix[2][1] * d[1] + worldMatrix[2][2] * d[2]
			);
			glm::vec3 scale2 = worldScale * worldScale;
			return scaledDir * glm::vec3(1./ scale2.x,1./ scale2.y,1./ scale2.z);
		}
		
		inline glm::vec3 rotationDirection(const glm::vec3& d)const {
			glm::vec3 scaledDir = glm::vec3(
				worldMatrix[0][0] * d[0] + worldMatrix[1][0] * d[1] + worldMatrix[2][0] * d[2],
				worldMatrix[0][1] * d[0] + worldMatrix[1][1] * d[1] + worldMatrix[2][1] * d[2],
				worldMatrix[0][2] * d[0] + worldMatrix[1][2] * d[1] + worldMatrix[2][2] * d[2]
			);
			glm::vec3 scale2 = worldScale;
			return scaledDir * glm::vec3(1. / scale2.x, 1. / scale2.y, 1. / scale2.z);
		}
		/// <summary>
		/// obtain direction in local space, scale factor has no effect
		/// </summary>
		/// <param name="d"></param>
		/// <returns></returns>
		inline glm::vec3 rotationInverseDirection(const glm::vec3& d)const {
			glm::vec3 scaledDir = glm::vec3(
				worldMatrix[0][0] * d[0] + worldMatrix[0][1] * d[1] + worldMatrix[0][2] * d[2],
				worldMatrix[1][0] * d[0] + worldMatrix[1][1] * d[1] + worldMatrix[1][2] * d[2],
				worldMatrix[2][0] * d[0] + worldMatrix[2][1] * d[1] + worldMatrix[2][2] * d[2]
			);
			glm::vec3 scale2 = worldScale;
			return scaledDir * glm::vec3(1. / scale2.x, 1. / scale2.y, 1. / scale2.z);
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