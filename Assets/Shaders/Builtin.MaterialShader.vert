#version 460

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout (location = 0) out vec2 vTexCoord;

layout (set = 0, binding = 0) uniform GlobalUBO {
	mat4 projection;
	mat4 view;
} globalUbo;

layout (push_constant) uniform PushConstants {
	mat4 model;
} pushConstants;

void main() {
	vTexCoord = aTexCoord;
	gl_Position = globalUbo.projection * globalUbo.view * pushConstants.model * vec4(aPos, 1.0);
}