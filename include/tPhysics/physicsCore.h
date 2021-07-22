#pragma once
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtx/norm.hpp>
namespace tPhysics {
	using real = float;
	using Vector3 = glm::vec<3, real>;
	using Vector4 = glm::vec<4, real>;
	using Quaternion = glm::quat;
	using Mat4 = glm::mat4;
	using Mat3 = glm::mat3;
	inline real dot(const Vector3& a, const Vector3& b) {
		return glm::dot(a, b);
	}
	inline real magnitude(const Vector3& a) {
		return glm::length(a);
	}
	static real sleepEpsilon=0.3;
}