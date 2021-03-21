#include"Noise.h"
#include"utils.hpp"

namespace tEngine {
	using namespace glm;
	using  vk::su::clamp;
	float RadicalInverse(int Base, int i)
	{
		float Digit, Radical, Inverse;
		Digit = Radical = 1.0 / float(Base);
		Inverse = 0.0;
		while (i != 0)
		{
			// i余Base求出i在"Base"进制下的最低位的数
			// 乘以Digit将这个数镜像到小数点右边
			Inverse += Digit * float(i % Base);
			Digit *= Radical;

			// i除以Base即可求右一位的数
			i /= Base;
		}
		return Inverse;
	}
	float IntegerRadicalInverse(int Base, int i)
	{
		int numPoints, inverse;
		numPoints = 1;
		// 此循环将i在"Base"进制下的数字左右Flip
		for (inverse = 0; i > 0; i /= Base)
		{
			inverse = inverse * Base + (i % Base);
			numPoints = numPoints * Base;
		}

		// 除以Digit将这个数镜像到小数点右边
		return float(inverse) / float(numPoints);
	}

	int ihash(int n) {
		n = (n << 13) ^ n;
		return (n * (n * n * 15731 + 789221) + 1376312589) & 2147483647;
	}
	ivec2 ihash(ivec2 n)
	{
		n = (n << 13) ^ n;
		return (n * (n * n * 15731 + 789221) + 1376312589) & 2147483647;
	}

	ivec3 ihash(ivec3 n)
	{
		n = (n << 13) ^ n;
		return (n * (n * n * 15731 + 789221) + 1376312589) & 2147483647;
	}
	vec2 frand(ivec2 n)
	{
		return vec2(ihash(n)) / 2147483647.f;
	}

	vec3 frand(ivec3 n)
	{
		return vec3(ihash(n)) / 2147483647.f;
	}

	float hash(vec3 p)  // replace this by something better
	{
		p = 50.0f * fract(p * 0.3183099f + vec3(0.71, 0.113, 0.419));
		return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
	}
	float hash(vec2 p)
	{
		p = 50.0f * fract(p * 0.3183099f);
		return fract(p.x * p.y * (p.x + p.y));
	}

	float hash(float n)
	{
		return fract(n * 17.0 * fract(n * 0.3183099));
	}

	vec2 hash2D(float n) { return fract(sin(vec2(n, n + 1.0)) * vec2(43758.5453123, 22578.1459123)); }


	vec2 hash2D(vec2 p)
	{
		const vec2 k = vec2(0.3183099, 0.3678794);
		p = p * k + vec2(k.y,k.x);
		return fract(16.0f * k * fract(p.x * p.y * (p.x + p.y)));
	}


	//vec2 g( vec2 n ) { return sin(n.x*n.y+vec2(0,1.571)); } // if you want the gradients to lay on a circle
	vec3 hash3D(vec3 p) // replace this by something better. really. do
	{
		p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
			dot(p, vec3(269.5, 183.3, 246.1)),
			dot(p, vec3(113.5, 271.9, 124.6)));

		return fract(sin(p) * 43758.5453123f);
	}



	// 3D random number generator inspired by PCGs (permuted congruential generator)
	// Using a **simple** Feistel cipher in place of the usual xor shift permutation step
	// @param v = 3D integer coordinate
	// @return three elements w/ 16 random bits each (0-0xffff).
	// ~8 ALU operations for result.x    (7 mad, 1 >>)
	// ~10 ALU operations for result.xy  (8 mad, 2 >>)
	// ~12 ALU operations for result.xyz (9 mad, 3 >>)
	uvec3 Rand3DPCG16(ivec3 p)
	{
		// taking a signed int then reinterpreting as unsigned gives good behavior for negatives
		uvec3 v = uvec3(p);

		// Linear congruential step. These LCG constants are from Numerical Recipies
		// For additional #'s, PCG would do multiple LCG steps and scramble each on output
		// So v here is the RNG state
		v = v * 1664525u + 1013904223u;

		// PCG uses xorshift for the final shuffle, but it is expensive (and cheap
		// versions of xorshift have visible artifacts). Instead, use simple MAD Feistel steps
		//
		// Feistel ciphers divide the state into separate parts (usually by bits)
		// then apply a series of permutation steps one part at a time. The permutations
		// use a reversible operation (usually ^) to part being updated with the result of
		// a permutation function on the other parts and the key.
		//
		// In this case, I'm using v.x, v.y and v.z as the parts, using + instead of ^ for
		// the combination function, and just multiplying the other two parts (no key) for 
		// the permutation function.
		//
		// That gives a simple mad per round.
		v.x += v.y * v.z;
		v.y += v.z * v.x;
		v.z += v.x * v.y;
		v.x += v.y * v.z;
		v.y += v.z * v.x;
		v.z += v.x * v.y;

		// only top 16 bits are well shuffled
		return v >> 16u;
	}
#define MGradientMask uvec3(0x8000, 0x4000, 0x2000)
#define MGradientScale vec3(1. /float( 0x4000), 1. / float(0x2000), 1. / float(0x1000))
	// Modified noise gradient term
	// @param seed - random seed for integer lattice position
	// @param offset - [-1,1] offset of evaluation point from lattice point
	// @return gradient direction (xyz) and contribution (w) from this lattice point
	vec4 MGradient(int seed, vec3 offset)
	{
		uint rand = Rand3DPCG16(ivec3(seed, 0, 0)).x;
		vec3 direction = vec3(rand & MGradientMask.x, rand & MGradientMask.y, rand & MGradientMask.z) * MGradientScale - vec3(1, 1, 1);
		return vec4(direction, dot(direction, offset));
	}
	float SmoothStepC1(float edge0, float edge1, float x)
	{
		
		float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		return clamp(t * t * t * (3. - 2. * t), 0., 1.);

	}
	float SmoothStepC1(float t) {
		return clamp(t * t * t * (3. - 2. * t), 0., 1.);
	}
	float SmoothStepC2(float t)
	{
		return clamp(t * t * t * (t * (t * 6. - 15.) + 10.), 0., 1.);
	}
	float DirevSmoothStepC2(float t)
	{   //t=clamp(t,0,1);
		return clamp(t * t * (30. + t * (30. * t - 60.)), 0., 1.);
	}

	float pulse(float lowCut, float hCut, float x) {
		return SmoothStepC1(lowCut - 0.1, lowCut, x) - SmoothStepC1(hCut, hCut + 0.1, x);
	}
	float lowPass(float cutoff, float x) {
		return x * (1.0 - SmoothStepC1(cutoff, cutoff + 0.1, x));
	}
	float highPass(float cutoff, float x) {
		return x * SmoothStepC1(cutoff - 0.1, cutoff, x);
	}
	float bandPass(float lowCut, float hCut, float x) {
		return x * pulse(lowCut, hCut, x);
	}
	float bandPassNormal(float lowCut, float hCut, float x) {
		return SmoothStepC1(lowCut, hCut, x) - SmoothStepC1(hCut, hCut + 0.1, x);
	}
	float PerlinNoise3D(vec3 p)
	{
		vec3 pi = floor(p);
		vec3 pf = p - pi;

		vec3 w = pf * pf * (3.0f - 2.0f * pf);

		return mix(
			mix(
				mix(dot(pf - vec3(0, 0, 0), hash3D(pi + vec3(0, 0, 0))),
					dot(pf - vec3(1, 0, 0), hash3D(pi + vec3(1, 0, 0))),
					w.x),
				mix(dot(pf - vec3(0, 0, 1), hash3D(pi + vec3(0, 0, 1))),
					dot(pf - vec3(1, 0, 1), hash3D(pi + vec3(1, 0, 1))),
					w.x),
				w.z),
			mix(
				mix(dot(pf - vec3(0, 1, 0), hash3D(pi + vec3(0, 1, 0))),
					dot(pf - vec3(1, 1, 0), hash3D(pi + vec3(1, 1, 0))),
					w.x),
				mix(dot(pf - vec3(0, 1, 1), hash3D(pi + vec3(0, 1, 1))),
					dot(pf - vec3(1, 1, 1), hash3D(pi + vec3(1, 1, 1))),
					w.x),
				w.z),
			w.y);
	}

	float SimplexNoise2D(vec2 p)
	{
		const float K1 = 0.366025404; // (sqrt(3)-1)/2;
		const float K2 = 0.211324865; // (3-sqrt(3))/6;

		vec2  i = floor(p + (p.x + p.y) * K1);
		vec2  a = p - i + (i.x + i.y) * K2;
		float m = step(a.y, a.x);
		vec2  o = vec2(m, 1.0 - m);
		vec2  b = a - o + K2;
		vec2  c = a - 1.0f + 2.0f * K2;
		vec3  h = max(0.5f - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0f);
		vec3  n = h * h * h * h * vec3(dot(a, hash2D(i + 0.0f)), dot(b, hash2D(i + o)), dot(c, hash2D(i + 1.0f)));
		return dot(n, vec3(70.0));
	}
	float WaveNoise2D(vec2 p)
	{
		const float kF = 3.1415927;  // make 6 to see worms

		vec2 i = floor(p);
		vec2 f = fract(p);
		f = f * f * (3.0f - 2.0f * f);
		return mix(mix(sin(kF * dot(p, hash2D(i + vec2(0, 0)))),
			sin(kF * dot(p, hash2D(i + vec2(1, 0)))), f.x),
			mix(sin(kF * dot(p, hash2D(i + vec2(0, 1)))),
				sin(kF * dot(p, hash2D(i + vec2(1, 1)))), f.x), f.y);
	}



	// return gradient noise (in x) and its derivatives (in yz)
	vec3 GradientNoise2D( vec2 p)
	{
		vec2 i = floor(p);
		vec2 f = fract(p);

#if 1
		// quintic interpolation
		vec2 u = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
		vec2 du = 30.0f * f * f * (f * (f - 2.0f) + 1.0f);
#else
		// cubic interpolation
		vec2 u = f * f * (3.0 - 2.0 * f);
		vec2 du = 6.0 * f * (1.0 - f);
#endif    

		vec2 ga = hash2D(i + vec2(0.0, 0.0));
		vec2 gb = hash2D(i + vec2(1.0, 0.0));
		vec2 gc = hash2D(i + vec2(0.0, 1.0));
		vec2 gd = hash2D(i + vec2(1.0, 1.0));

		float va = dot(ga, f - vec2(0.0, 0.0));
		float vb = dot(gb, f - vec2(1.0, 0.0));
		float vc = dot(gc, f - vec2(0.0, 1.0));
		float vd = dot(gd, f - vec2(1.0, 1.0));

		return vec3(va + u.x * (vb - va) + u.y * (vc - va) + u.x * u.y * (va - vb - vc + vd),   // value
			ga + u.x * (gb - ga) + u.y * (gc - ga) + u.x * u.y * (ga - gb - gc + gd) +  // derivatives
			du * (vec2(u.y,u.x) * (va - vb - vc + vd) + vec2(vb, vc) - va));
	}
	// return value noise (in x) and its derivatives (in yzw)
	vec4 ValueNoise3D( vec3 x)
	{
		vec3 i = floor(x);
		vec3 w = fract(x);

#if 1
		// quintic interpolation
		vec3 u = w * w * w * (w * (w * 6.0f - 15.0f) + 10.0f);
		vec3 du = 30.0f * w * w * (w * (w - 2.0f) + 1.0f);
#else
		// cubic interpolation
		vec3 u = w * w * (3.0 - 2.0 * w);
		vec3 du = 6.0 * w * (1.0 - w);
#endif    


		float a = hash(i + vec3(0.0, 0.0, 0.0));
		float b = hash(i + vec3(1.0, 0.0, 0.0));
		float c = hash(i + vec3(0.0, 1.0, 0.0));
		float d = hash(i + vec3(1.0, 1.0, 0.0));
		float e = hash(i + vec3(0.0, 0.0, 1.0));
		float f = hash(i + vec3(1.0, 0.0, 1.0));
		float g = hash(i + vec3(0.0, 1.0, 1.0));
		float h = hash(i + vec3(1.0, 1.0, 1.0));

		float k0 = a;
		float k1 = b - a;
		float k2 = c - a;
		float k3 = e - a;
		float k4 = a - b - c + d;
		float k5 = a - c - e + g;
		float k6 = a - b - e + f;
		float k7 = -a + b + c - d + e - f - g + h;

		return vec4(k0 + k1 * u.x + k2 * u.y + k3 * u.z + k4 * u.x * u.y + k5 * u.y * u.z + k6 * u.z * u.x + k7 * u.x * u.y * u.z,
			du * vec3(k1 + k4 * u.y + k6 * u.z + k7 * u.y * u.z,
				k2 + k5 * u.z + k4 * u.x + k7 * u.z * u.x,
				k3 + k6 * u.x + k5 * u.y + k7 * u.x * u.y));
	}
	// return gradient noise (in x) and its derivatives (in yzw)
	vec4 GradientNoise3D( vec3 x)
	{
		// grid
		vec3 i = floor(x);
		vec3 w = fract(x);

#if 1
		// quintic interpolant
		vec3 u = w * w * w * (w * (w * 6.0f - 15.0f) + 10.0f);
		vec3 du = 30.0f * w * w * (w * (w - 2.0f) + 1.0f);
#else
		// cubic interpolant
		vec3 u = w * w * (3.0 - 2.0 * w);
		vec3 du = 6.0 * w * (1.0 - w);
#endif    

		// gradients
		vec3 ga = hash3D(i + vec3(0.0, 0.0, 0.0));
		vec3 gb = hash3D(i + vec3(1.0, 0.0, 0.0));
		vec3 gc = hash3D(i + vec3(0.0, 1.0, 0.0));
		vec3 gd = hash3D(i + vec3(1.0, 1.0, 0.0));
		vec3 ge = hash3D(i + vec3(0.0, 0.0, 1.0));
		vec3 gf = hash3D(i + vec3(1.0, 0.0, 1.0));
		vec3 gg = hash3D(i + vec3(0.0, 1.0, 1.0));
		vec3 gh = hash3D(i + vec3(1.0, 1.0, 1.0));

		// projections
		float va = dot(ga, w - vec3(0.0, 0.0, 0.0));
		float vb = dot(gb, w - vec3(1.0, 0.0, 0.0));
		float vc = dot(gc, w - vec3(0.0, 1.0, 0.0));
		float vd = dot(gd, w - vec3(1.0, 1.0, 0.0));
		float ve = dot(ge, w - vec3(0.0, 0.0, 1.0));
		float vf = dot(gf, w - vec3(1.0, 0.0, 1.0));
		float vg = dot(gg, w - vec3(0.0, 1.0, 1.0));
		float vh = dot(gh, w - vec3(1.0, 1.0, 1.0));

		// interpolations
		return vec4(va + u.x * (vb - va) + u.y * (vc - va) + u.z * (ve - va) + u.x * u.y * (va - vb - vc + vd) + u.y * u.z * (va - vc - ve + vg) + u.z * u.x * (va - vb - ve + vf) + (-va + vb + vc - vd + ve - vf - vg + vh) * u.x * u.y * u.z,    // value
			ga + u.x * (gb - ga) + u.y * (gc - ga) + u.z * (ge - ga) + u.x * u.y * (ga - gb - gc + gd) + u.y * u.z * (ga - gc - ge + gg) + u.z * u.x * (ga - gb - ge + gf) + (-ga + gb + gc - gd + ge - gf - gg + gh) * u.x * u.y * u.z +   // derivatives
			du * (vec3(vb, vc, ve) - va + vec3(u.y,u.z,u.x) * vec3(va - vb - vc + vd, va - vc - ve + vg, va - vb - ve + vf) + vec3(u.z, u.x, u.y) * vec3(va - vb - ve + vf, va - vb - vc + vd, va - vc - ve + vg) + vec3(u.y, u.z, u.x) * vec3(u.z, u.x, u.y) * (-va + vb + vc - vd + ve - vf - vg + vh)));
	}
	//For FBM example
	float fbm2D( vec2 x,  float H, int numOctaves)//H=0.5
	{
		float G = exp2(-H);
		float f = 1.0;
		float a = 1.0;
		float t = 0.0;
		for (int i = 0; i < numOctaves; i++)
		{
			t += a * GradientNoise2D(f * x).x;
			f *= 2.0;
			a *= G;
		}
		return t;
	}
	float fbm3D( vec3 x,  float H, int numOctaves)//H=0.5
	{
		float G = exp2(-H);
		float f = 1.0;
		float a = 1.0;
		float t = 0.0;
		for (int i = 0; i < numOctaves; i++)
		{
			t += a * GradientNoise3D(f * x).x;
			f *= 2.0;
			a *= G;
		}
		return t;
	}
	
	vec3 Curl3D(vec3 uv)
	{
		auto yzw = [](vec4 v) {return vec3(v.y,v.z,v.w); };
		vec3 eps = vec3(1, 0, 0);
		float n1, n2, a, b;
		vec3 noise;
		vec3 phi = yzw( GradientNoise3D(uv));
		vec3 phix = yzw(GradientNoise3D(uv + eps));
		vec3 phiy = yzw(GradientNoise3D(uv + vec3(eps.y,eps.x,eps.z)));
		vec3 phiz = yzw(GradientNoise3D(uv + vec3(eps.y, eps.z, eps.x)));
		noise.x = (phiy.z - phi.z) - (phiz.y - phi.y);
		noise.y = (phiz.x - phi.x) - (phix.z - phi.z);
		noise.z = (phix.y - phi.y) - (phiy.x - phi.x);
	

		return noise;

	}
	/* skew constants for 3d simplex functions */

	/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
	vec3 random3(vec3 c) {
		float j = 4096.0 * sin(dot(c, vec3(17.0, 59.4, 15.0)));
		vec3 r;
		r.z = fract(512.0 * j);
		j *= .125;
		r.x = fract(512.0 * j);
		j *= .125;
		r.y = fract(512.0 * j);
		return r - 0.5f;
	}
	/* 3d simplex noise */
	float simplex3d(vec3 p) {
		const float F3 = 0.3333333;
		const float G3 = 0.1666667;
		/* 1. find current tetrahedron T and it's four vertices */
		/* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
		/* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/
		
		/* calculate s and x */
		vec3 s = floor(p + dot(p, vec3(F3)));
		vec3 x = p - s + dot(s, vec3(G3));

		/* calculate i1 and i2 */
		vec3 e = step(vec3(0.0), x - vec3(e.y,e.z,e.x));
		vec3 i1 = e * (1.0f - vec3(e.z,e.x,e.y));
		vec3 i2 = 1.0f - vec3(e.z, e.x, e.y) * (1.0f - e);

		/* x1, x2, x3 */
		vec3 x1 = x - i1 + G3;
		vec3 x2 = x - i2 + 2.0f * G3;
		vec3 x3 = x - 1.0f+ 3.0f * G3;

		/* 2. find four surflets and store them in d */
		vec4 w, d;

		/* calculate surflet weights */
		w.x = dot(x, x);
		w.y = dot(x1, x1);
		w.z = dot(x2, x2);
		w.w = dot(x3, x3);

		/* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
		w = max(0.6f - w, 0.0f);

		/* calculate surflet components */
		d.x = dot(random3(s), x);
		d.y = dot(random3(s + i1), x1);
		d.z = dot(random3(s + i2), x2);
		d.w = dot(random3(s + 1.0f), x3);

		/* multiply d by w^4 */
		w *= w;
		w *= w;
		d *= w;

		/* 3. return the sum of the four surflets */
		return dot(d, vec4(52.0));
	}



	/* directional artifacts can be reduced by rotating each octave */
	float simplex3d_fractal(vec3 m) {
		/* const matrices for 3d rotation */
		const mat3 rot1 = mat3(-0.37, 0.36, 0.85, -0.14, -0.93, 0.34, 0.92, 0.01, 0.4);
		const mat3 rot2 = mat3(-0.55, -0.39, 0.74, 0.33, -0.91, -0.24, 0.77, 0.12, 0.63);
		const mat3 rot3 = mat3(-0.71, 0.52, -0.47, -0.08, -0.72, -0.68, -0.7, -0.45, 0.56);
		return   0.5333333 * simplex3d(m * rot1)
			+ 0.2666667 * simplex3d(2.0f * m * rot2)
			+ 0.1333333 * simplex3d(4.0f * m * rot3)
			+ 0.0666667 * simplex3d(8.0f * m);
	}
	float fade(float t) { return t * t * t * (t * (6. * t - 15.) + 10.); }

	float grad(float hash, float p)
	{
		int i = int(1e4 * hash);
		return (i & 1) == 0 ? p : -p;
	}

	float perlinNoise1D(float p)
	{
		float pi = floor(p);
		float pf = p - pi;
		float w = fade(pf);
		return mix(grad(hash(pi), pf), grad(hash(pi + 1.0), pf - 1.0), w) * 2.0;
	}
}