#version 460

layout (location = 0) in vec3 aPos;

layout (set = 0, binding = 0) uniform globalUniformObject {
	mat4 view;
	mat4 projection;
	vec4 color;
} globalUbo;

layout (location = 0) out vec4 vertexColor;

void main() {
	gl_Position = globalUbo.projection * globalUbo.view * vec4(aPos, 1.0);
	vertexColor = globalUbo.color;
}