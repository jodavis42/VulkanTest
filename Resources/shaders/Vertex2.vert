#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform PerCameraData {
    mat4 view;
    mat4 proj;

} cameraData;

layout(binding = 1) uniform PerObjectData {
    mat4 model;
} objData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUv;

void main() {
    gl_Position = cameraData.proj * cameraData.view * objData.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragUv = uv;
}