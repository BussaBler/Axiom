#version 460

layout (set = 0, binding = 1) uniform MaterialUniformObject {
	vec4 diffuseColor;
} materialUbo;

layout (set = 0, binding = 2) uniform sampler2D diffuseTexture;

layout (location = 0) in vec2 vTexCoord;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = materialUbo.diffuseColor * texture(diffuseTexture, vTexCoord);
}