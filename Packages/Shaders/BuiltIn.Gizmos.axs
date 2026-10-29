#type vertex
#version 460
#pragma shader_stage(vertex)

#include "Input.Mesh.axsinc"

#include "RS.GlobalData.axsinc"

layout(push_constant, std430) uniform PushConstants {
    mat4 uModel;
    vec4 uColor;
} pushConstants;

layout(location = 0) out vec4 vColor;

void main() {
    vColor = pushConstants.uColor;
    gl_Position = globalData.uProjection * globalData.uView * pushConstants.uModel * vec4(aPosition, 1.0);
}

#type fragment
#version 460
#pragma shader_stage(fragment)

layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vColor;
}
