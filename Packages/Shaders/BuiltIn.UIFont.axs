#type vertex
#version 460
#pragma shader_stage(vertex)

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUv;
layout (location = 2) in vec4 aColor;

layout(push_constant, std430) uniform PushConstants {
    mat4 proj;
};

layout (location = 0) out vec2 vUv;
layout (location = 1) out vec4 vColor;

void main() {
    gl_Position = proj * vec4(aPos, 0.0, 1.0);
    vUv = aUv;
    vColor = aColor;
}

#type fragment
#version 460 core
#pragma shader_stage(fragment)

layout(location = 0) in vec2 vUv;
layout(location = 1) in vec4 vColor;

layout(set = 0, binding = 0) uniform texture2D uFontTexture;
layout(set = 0, binding = 1) uniform sampler uFontSampler;

layout(location = 0) out vec4 oColor;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec3 msd = texture(sampler2D(uFontTexture, uFontSampler), vUv).rgb;
    float sd = median(msd.r, msd.g, msd.b) - 0.5;
    
    vec2 texSize = vec2(textureSize(sampler2D(uFontTexture, uFontSampler), 0));
    vec2 dx = dFdx(vUv) * texSize;
    vec2 dy = dFdy(vUv) * texSize;
    
    float toPixels = 4.0 * inversesqrt(0.5 * (dot(dx, dx) + dot(dy, dy)));
    float screenPxDistance = toPixels * sd;
    
    float alpha = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    
    oColor = vec4(vColor.rgb * alpha, vColor.a * alpha);
}