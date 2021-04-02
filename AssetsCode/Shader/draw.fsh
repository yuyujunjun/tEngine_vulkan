#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec2 uv;
layout(location=1)in vec4 color;
layout(location=2) in vec3 normal;
layout(location=3)in vec3 worldPosition;
layout (location = 0) out vec4 outColor;

void main() {
//   outColor = vec4(texture(_MainTex,uv));
	vec3 abeldo=vec3(1,1,1);
	float LdotV=abs(dot(normalize(normal),vec3(0,0,1)));
	outColor=  vec4(LdotV*abeldo*0.7+abeldo*0.3,1);
}
