#version 460

layout (location = 0) in vec2 vTexCoord;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform localUniformObject {
	vec4 diffuseColor;
} objectData;
layout (set = 1, binding = 1) uniform sampler2D diffuseSampler;

void main() {
	outColor = objectData.diffuseColor * texture(diffuseSampler, vTexCoord);
}