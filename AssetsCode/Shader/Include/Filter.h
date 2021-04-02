#include"Global.h"
layout(set = 0, binding = 1)uniform sampler2D _MainTex;//16 is the minmum

layout(set = 1, binding = 1)uniform _MainTexTexelSize
{
	vec4 _MainTex_TexelSize;
};
layout(set = 1, binding = 2)uniform BrightnessProperty
{
	vec4 _Filter;
	float intensity;
	float radius;
	float density;
};

// RGBM encoding/decoding
vec4 EncodeHDR(vec3 rgb)
{
	rgb *= 0.125;
	float m = max(max(rgb.r, rgb.g), max(rgb.b, 1e-6));
	m = ceil(m * 255) / 255;
	mediump vec4 result = vec4(rgb / m, m);
	return result;
}

vec3 DecodeHDR(mediump vec4 rgba)
{
	return rgba.rgb * rgba.a * 8;
}

//vec3 Prefilter(vec3 c) {
//	float brightness = max(c.r, max(c.g, c.b));
//	float soft = brightness - _Filter.y;
//	soft = clamp(soft, 0, _Filter.z);
//	soft = soft * soft * _Filter.w;
//	float contribution = max(soft, brightness - _Filter.x);
//	contribution /= max(brightness, 0.00001);
//	vec3 color = c * contribution;
//	return color;
//}



// 3-tap median filter
mediump vec3 Median(mediump vec3 a, mediump vec3 b, mediump vec3 c)
{
	return a + b + c - min(min(a, b), c) - max(max(a, b), c);
}


