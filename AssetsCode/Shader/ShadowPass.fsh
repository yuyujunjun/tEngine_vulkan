#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) in vec3 worldPosition;
void main() {
  // gl_FragDepth=-worldPosition.z;
}
