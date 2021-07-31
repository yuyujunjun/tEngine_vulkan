#pragma once
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtx/norm.hpp>
#include<math.h>
namespace tEngine {
	using real = float;
	using Vector3 = glm::vec<3, real>;
	using Vector4 = glm::vec<4, real>;
	using Quaternion = glm::quat;
	using Mat4 = glm::mat4;
	using Mat3 = glm::mat3;
#define MAXREAL std::numeric_limits<real>().max()
#define MINREAL std::numeric_limits<real>().lowest()
#define CCD_EPS  1.192092896e-07F
	inline bool isZero(real x) {
		return abs(x) < CCD_EPS;
	}
	inline bool isZero(const Vector3& v) {
		return isZero(v.x) && isZero(v.y) && isZero(v.z);
	}
	inline bool ccdEqual(real  x, real y) {
		real ab;
		real a, b;

		ab = abs(x - y);
		if (isZero(ab))
			return true;

		a = abs(x);
		b = abs(y);
		if (b > a) {
			return ab < CCD_EPS* b;
		}
		else {
			return ab < CCD_EPS* a;
		}
	}
	inline bool ccdEqual(const Vector3& a, const Vector3& b) {
		return ccdEqual(a.x, b.x) && ccdEqual(a.y, b.y) && ccdEqual(a.z, b.z);
	}
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
		if (isZero(a))return 0;
		return a < 0 ? -1 : 1;
	}
	inline Mat3 absMat(const Mat3& mat) {
		return Mat3(abs(mat[0]), abs(mat[1]), abs(mat[2]));
	}
	inline Mat3 createSkewMat3(const Vector3& v) {
		return Mat3(0,v.z,-v.y,
					-v.z,0,v.x,
					v.y,-v.x,0);
	}
	static real sleepEpsilon=0.3;

}