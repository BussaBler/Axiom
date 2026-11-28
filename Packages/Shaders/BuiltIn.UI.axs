#type vertex
#version 460
#pragma shader_stage(vertex)

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec4 aData; // rect: size
layout(location = 4) in vec4 aRadii;
layout(location = 5) in uint aTextureIndex;
layout(location = 6) in uint aPrimitive; // 0 = Rect, 1 = Glyph, 2 = Image

layout(push_constant, std430) uniform PushConstants {
    mat4 proj;
};

layout(location = 0) out vec2 vUv;
layout(location = 1) out vec4 vColor;
layout(location = 2) out vec2 vSize;
layout(location = 3) out vec4 vRadii;
layout(location = 4) out flat uint vTextureIndex;
layout(location = 5) out flat uint vPrimitive;
layout(location = 6) out flat uint vSamplerIndex;

void main() {
    gl_Position = proj * vec4(aPos, 0.0, 1.0);
    vUv = aUv;
    if (aPrimitive == 0) {
        vUv = (aUv - 0.5) * aData.xy;
    }
    vColor = aColor;
    vSize = aData.xy * 0.5;
    vRadii = aRadii;
    vTextureIndex = aTextureIndex;
    vPrimitive = aPrimitive;
    vSamplerIndex = (aPrimitive == 1u) ? 1u : 0u;
}

#type fragment
#version 460
#pragma shader_stage(fragment)

layout(location = 0) in vec2 vUv;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vSize;
layout(location = 3) in vec4 vRadii;
layout(location = 4) in flat uint vTextureIndex;
layout(location = 5) in flat uint vPrimitive;
layout(location = 6) in flat uint vSamplerIndex;

#extension GL_EXT_nonuniform_qualifier : enable
layout(set = 0, binding = BINDING_BINDLESS_TEXTURE) uniform texture2D globalTextures[];
layout(set = 0, binding = BINDING_BINDLESS_TEXTURE_SAMPLERS) uniform sampler globalSamplers[];

layout(location = 0) out vec4 oColor;

float getRadius(vec2 p, vec4 radii) {
    if (p.x > 0.0) {
        return (p.y > 0.0) ? radii.y : radii.w;
    } else {
        return (p.y > 0.0) ? radii.x : radii.z;
    }
}

float sdRoundedBox(vec2 p, vec2 b, vec4 r) {
    float radius = getRadius(p, r);
    vec2 q = abs(p) - b + radius;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    if (vPrimitive == 0) {
        float dist = sdRoundedBox(vUv, vSize, vRadii);
        float alpha = 1.0 - smoothstep(-1.0, 1.0, dist);

        oColor = vec4(vColor.rgb * alpha, vColor.a * alpha);
    }
    if (vPrimitive == 1) {
        vec3 msd = texture(sampler2D(globalTextures[nonuniformEXT(vTextureIndex)], globalSamplers[vSamplerIndex]), vUv).rgb;
        float sd = median(msd.r, msd.g, msd.b) - 0.5;

        vec2 texSize = vec2(textureSize(sampler2D(globalTextures[nonuniformEXT(vTextureIndex)], globalSamplers[vSamplerIndex]), 0));
        vec2 dx = dFdx(vUv) * texSize;
        vec2 dy = dFdy(vUv) * texSize;

        float toPixels = 4.0 * inversesqrt(0.5 * (dot(dx, dx) + dot(dy, dy)));
        float screenPxDistance = toPixels * sd;

        float alpha = clamp(screenPxDistance + 0.5, 0.0, 1.0);

        oColor = vec4(vColor.rgb * alpha, vColor.a * alpha);
    }
    if (vPrimitive == 2) {
        oColor = vColor * texture(sampler2D(globalTextures[nonuniformEXT(vTextureIndex)], globalSamplers[vSamplerIndex]), vUv);
    }
}
