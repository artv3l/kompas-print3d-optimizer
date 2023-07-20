#ifndef SHADERS_HPP
#define SHADERS_HPP

inline const char* VERTEX_SHADER_CODE = R"glsl(
#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 u_modelview;
uniform mat4 u_projection;

out vec3 position;
out vec3 normal;

void main() {
    position = a_position;
    normal = a_normal;
    gl_Position = u_projection * u_modelview * vec4(a_position, 1.0f);
}

)glsl";

inline const char* FRAGMENT_SHADER_CODE = R"glsl(
#version 330

in vec3 position;
in vec3 normal;

uniform int u_mode;
uniform vec3 u_printSurfaceNormal;
uniform float u_printSurfaceD;
uniform float u_layerHeight;
uniform float u_overhangThreshold;
uniform float u_epsilon;

out vec4 FragColor;

void main() {
    vec3 color = vec3(0.0f, 0.0f, 0.0f);
    if (bool(u_mode & 0x04)) { // Нависания
        float angle = acos(dot(u_printSurfaceNormal, normal) / (length(u_printSurfaceNormal) * length(normal)));
        if (angle < u_overhangThreshold) {
            color.r = 1.0f;
        }
    }
    float dist = (dot(u_printSurfaceNormal, position) + u_printSurfaceD) / length(u_printSurfaceNormal);
    if (bool(u_mode & 0x01)) { // Слои везде
        if (abs(dist - (round(dist / u_layerHeight) * u_layerHeight)) < u_epsilon) {
            color.r = 0.0f;
            color.g = 1.0f;
        }
    } else if (bool(u_mode & 0x02)) { // Слои у курсора
        // todo
    }
    FragColor = vec4(color, 0.3f);
}

)glsl";

#endif /* SHADERS_HPP */
