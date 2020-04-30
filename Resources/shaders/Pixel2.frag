#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(binding = 3) uniform sampler2D Texture;
layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform Material {
    vec4 Albedo;
    float scalar;
} material;

void main() {
    outColor = texture(Texture, fragUv);
    outColor.xyz *= material.Albedo.xyz * material.scalar;
}