#version 460

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout (set = 0, binding = 0) uniform GlobalUniformObject {
	mat4 view;
	mat4 projection;
} globalUbo;

layout (push_constant) uniform PushConstants {
	mat4 model;
} pushConstants;

layout (location = 0) out vec2 vTexCoord;

void main() {
	gl_Position = globalUbo.projection * globalUbo.view * pushConstants.model * vec4(aPos, 1.0);
	vTexCoord = aTexCoord;
}