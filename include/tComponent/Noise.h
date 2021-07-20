#pragma once
#include<glm.hpp>
namespace tEngine {
	using namespace glm;
	float RadicalInverse(int Base, int i);
	float IntegerRadicalInverse(int Base, int i);

	int ihash(int n);
	ivec2 ihash(ivec2 n);

	ivec3 ihash(ivec3 n);
	vec2 frand(ivec2 n);
	vec3 frand(ivec3 n);
	float hash(vec3 p);
	float hash(vec2 p);

	float hash(float n);

	vec2 hash2D(float n);


	vec2 hash2D(vec2 p);


	//vec2 g( vec2 n ) { return sin(n.x*n.y+vec2(0,1.571)); } // if you want the gradients to lay on a circle
	vec3 hash3D(vec3 p);



	// 3D random number generator inspired by PCGs (permuted congruential generator)
	// Using a **simple** Feistel cipher in place of the usual xor shift permutation step
	// @param v = 3D integer coordinate
	// @return three elements w/ 16 random bits each (0-0xffff).
	// ~8 ALU operations for result.x    (7 mad, 1 >>)
	// ~10 ALU operations for result.xy  (8 mad, 2 >>)
	// ~12 ALU operations for result.xyz (9 mad, 3 >>)
	uvec3 Rand3DPCG16(ivec3 p);
	;
	// Modified noise gradient term
	// @param seed - random seed for integer lattice position
	// @param offset - [-1,1] offset of evaluation point from lattice point
	// @return gradient direction (xyz) and contribution (w) from this lattice point
	vec4 MGradient(int seed, vec3 offset);
	float SmoothStepC1(float edge0, float edge1, float x);
	float SmoothStepC1(float t);
	float SmoothStepC2(float t);
	float DirevSmoothStepC2(float t);
	float pulse(float lowCut, float hCut, float x);
	float lowPass(float cutoff, float x);
	float highPass(float cutoff, float x);
	float bandPass(float lowCut, float hCut, float x);
	float bandPassNormal(float lowCut, float hCut, float x);
	float PerlinNoise3D(vec3 p);
	float SimplexNoise2D(vec2 p);
	float WaveNoise2D(vec2 p);


	// return gradient noise (in x) and its derivatives (in yz)
	vec3 GradientNoise2D(vec2 p);
	// return value noise (in x) and its derivatives (in yzw)
	vec4 ValueNoise3D(vec3 x);
	// return gradient noise (in x) and its derivatives (in yzw)
	vec4 GradientNoise3D(vec3 x);
	//For FBM example
	float fbm2D(vec2 x, float H, int numOctaves);//H=0.5;
	float fbm3D(vec3 x, float H, int numOctaves);//H=0.5;
	vec3 Curl3D(vec3 uv);
	/* skew constants for 3d simplex functions */

	/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
	vec3 random3(vec3 c);
	/* 3d simplex noise */
	float simplex3d(vec3 p);


	/* directional artifacts can be reduced by rotating each octave */
	float simplex3d_fractal(vec3 m);
	float fade(float t);

	float grad(float hash, float p);

	float perlinNoise1D(float p);
}