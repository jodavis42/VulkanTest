#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(binding = 3) uniform sampler2D texSampler;
layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform MaterialData {
    vec3 albedo;
} material;

void main() {
    outColor = texture(texSampler, fragUv);
    outColor.xyz *= material.albedo;
}