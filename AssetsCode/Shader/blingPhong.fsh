#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec2 uv;
layout(location=1)in vec4 color;
layout(location=2) in vec3 worldNormal;
layout(location=3)in vec3 worldPosition;
layout (location = 0) out vec4 outColor;
layout(set = 1, binding = 3)uniform sampler2D _MainTex;
layout(set = 0, binding = 1) uniform MaterialInfo {
	vec3 lightPos;
    float lightIntensity;
    vec3 uKd;float pad0;
    vec3 uKs;float pad1;
    vec3 cameraPos;

};

void main() {
//   outColor = vec4(texture(_MainTex,uv));
    vec3 color=pow(texture(_MainTex,uv).xyz,vec3(2.2));
    vec3 ambient=0.05*color;
    
    vec3 lightDir=(lightPos-worldPosition);
    float distance_to_light=length(lightDir);
    lightDir=lightDir/distance_to_light;
    vec3 normal=normalize(worldNormal);
    float diff=max(dot(lightDir,normal),0.0);
    float light_atten_coff=lightIntensity/distance_to_light;
    vec3 diffuse=diff*light_atten_coff*color;

    vec3 viewDir=normalize(cameraPos-worldPosition);
    float spec=0.0;
    vec3 reflectDir=2*dot(normal,lightDir)*normal-lightDir;
    spec=pow(max(dot(viewDir,reflectDir),0.0),35.0);
    vec3 specular=uKs*light_atten_coff*spec;
	outColor=  vec4(pow(ambient+diffuse+specular,vec3(1.0/2.2)),1.0);
}
