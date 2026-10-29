#type vertex
#version 460
#pragma shader_stage(vertex)

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;

#include "RS.GlobalData.axsinc"

struct SpriteInstance {
    mat4 model;
    vec4 color;
    uint textureId;
    vec3 padding;
};

layout(std430, set = 1, binding = 0) readonly buffer InstanceBuffer {
    SpriteInstance instances[];
};

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vUv;
layout(location = 2) out flat uint vTextureSlotIndex;
layout(location = 3) out flat uint vSamplerIndex;

void main() {
    SpriteInstance instance = instances[gl_InstanceIndex];

    gl_Position = globalData.uProjection * globalData.uView * instance.model * vec4(aPos, 1.0);
    vColor = instance.color;
    vUv = aUv;
    vTextureSlotIndex = instance.textureId;
    vSamplerIndex = 0;
}

#type fragment
#version 460
#pragma shader_stage(fragment)

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUv;
layout(location = 2) in flat uint vTextureSlotIndex;
layout(location = 3) in flat uint vSamplerIndex;

#include "RS.BindlessTexture.axsinc"

layout(location = 0) out vec4 oColor;

void main() {
    vec4 texColor = texture(sampler2D(globalTextures[vTextureSlotIndex], globalSamplers[vSamplerIndex]), vUv);
    oColor = vColor * texColor;
}
