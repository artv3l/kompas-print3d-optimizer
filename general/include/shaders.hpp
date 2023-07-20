#ifndef SHADERS_HPP
#define SHADERS_HPP

inline const char* VERTEX_SHADER_CODE = R"glsl(
#version 330

layout (location = 0) in vec3 a_pos;

uniform mat4 u_modelview;
uniform mat4 u_projection;

out vec3 globalPosition;

void main() {
    globalPosition = a_pos;
   gl_Position = u_projection * u_modelview * vec4(a_pos, 1.0f);
}

)glsl";

inline const char* FRAGMENT_SHADER_CODE = R"glsl(
#version 330

in vec3 globalPosition;

uniform int u_mode;
uniform vec3 u_printSurfaceNormal;
uniform float u_printSurfaceD;
uniform float u_layerHeight;
uniform float u_epsilon;

out vec4 FragColor;

void main() {
    vec3 color = vec3(0.0f, 0.0f, 0.0f);
    if (bool(u_mode & 0x04)) { // Нависания
        color.r = 1.0f;
    }
    float dist = (dot(u_printSurfaceNormal, globalPosition) + u_printSurfaceD) / length(u_printSurfaceNormal);
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
