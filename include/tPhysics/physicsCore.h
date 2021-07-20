#pragma once
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtx/norm.hpp>
namespace tPhysics {
	using real = float;
	using Vector3 = glm::vec<3, real>;
	real dot(const Vector3& a, const Vector3& b) {
		return glm::dot(a, b);
	}
	real magnitude(const Vector3& a) {
		return glm::length(a);
	}
}