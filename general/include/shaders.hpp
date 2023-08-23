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
    /* Смещаем полигоны в направлении нормали, чтобы не появлялись артефакты при вращении.
       Они появляются, потому что мы рисуем полигоны поверх уже нарисованных, но нет никаких гарантий, что наши полигоны отрисуются сверху. */
    gl_Position = u_projection * u_modelview * vec4(a_position + (a_normal * 0.03f), 1.0f);
}

)glsl";

inline const char* FRAGMENT_SHADER_CODE = R"glsl(
#version 330

#define PI 3.1415926538

layout(origin_upper_left) in vec4 gl_FragCoord;

in vec3 position;
in vec3 normal;

uniform int u_mode;
uniform vec3 u_printSurfaceNormal;
uniform float u_printSurfaceD;
uniform float u_layerHeight;
uniform float u_overhangThreshold;
uniform float u_lineWidth;
uniform bool u_isPrintSurface;
uniform vec2 u_mouseCoord;
uniform int u_mouseRadius;

out vec4 FragColor;

void main() {
    float epsilon = 0.001;
    vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float angle = acos(dot(u_printSurfaceNormal, normal) / length(u_printSurfaceNormal) / length(normal));
    if (bool(u_mode & 0x04)) { // нависания
        if ((angle < u_overhangThreshold) && !(abs(angle - u_overhangThreshold) < epsilon) && !u_isPrintSurface) {
            color.ra = vec2(1.0f, 0.3f);
        }
    }
    if (bool(u_mode & 0x03) && !(abs(angle) < epsilon) && !(abs(angle - PI) < epsilon)) { // слои
        float dist = (dot(u_printSurfaceNormal, position) + u_printSurfaceD) / length(u_printSurfaceNormal);
        if (abs(dist - (round(dist / u_layerHeight) * u_layerHeight)) < u_lineWidth * abs(sin(angle))) {
            if (bool(u_mode & 0x01)) { // везде
                color.rba = vec3(0.0f, 1.0f, 0.6f);
            } else { // у курсора
                if (length(u_mouseCoord - gl_FragCoord.xy) < u_mouseRadius) {
                    color.rba = vec3(0.0f, 1.0f, 0.6f);
                }
            }
        }
    }
    FragColor = color;
}

)glsl";

#endif /* SHADERS_HPP */
