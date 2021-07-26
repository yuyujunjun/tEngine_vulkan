#pragma once
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtx/norm.hpp>
#include<math.h>
namespace tPhysics {
	using real = float;
	using Vector3 = glm::vec<3, real>;
	using Vector4 = glm::vec<4, real>;
	using Quaternion = glm::quat;
	using Mat4 = glm::mat4;
	using Mat3 = glm::mat3;
#define MAXREAL std::numeric_limits<real>().max()
#define MINREAL std::numeric_limits<real>().min()
	inline real dot(const Vector3& a, const Vector3& b) {
		return glm::dot(a, b);
	}
	inline real magnitude(const Vector3& a) {
		return glm::length(a);
	}
	inline real max(real a, real b) {
		return a > b ? a : b;
	}
	inline real min(real a, real b) {
		return a > b ? b : a;
	}
	inline real max(const Vector3& a) {
		return max(max(a.x,a.y),a.z);
	}
	inline int sign(real a) {
		return a < 0 ? -1 : a>0;
	}
	inline Mat3 absMat(const Mat3& mat) {
		return Mat3(abs(mat[0]), abs(mat[1]), abs(mat[2]));
	}
	static real sleepEpsilon=0.3;
}