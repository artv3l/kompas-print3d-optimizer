#pragma once

#include <string>

inline const std::string VERTEX_SHADER_CODE = R"glsl(
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

inline const std::string FRAGMENT_SHADER_CODE = R"glsl(
#version 330

#define PI 3.1415926538

struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
};


const Light c_defaultLight = Light(
    vec3(0.3f, -0.3f, -1.0f),
    vec3(0.2f, 0.2f, 0.2f),
    vec3(0.5f, 0.5f, 0.5f)
);

const Material c_defaultMaterial = Material(
    vec3(0.8f, 0.8f, 0.8f),
    vec3(0.8f, 0.8f, 0.8f)
);

layout(origin_upper_left) in vec4 gl_FragCoord;

in vec3 position;
in vec3 normal;

uniform mat4 u_modelview;
uniform int u_mode;
uniform vec3 u_printSurfaceNormal;
uniform float u_printSurfaceD;
uniform float u_layerHeight;
uniform float u_overhangThreshold;
uniform float u_lineWidth;
uniform vec2 u_mouseCoord;
uniform int u_mouseRadius;

out vec4 FragColor;

void main() {
    float epsilon = 0.01;

    // ambient
    vec3 ambient = c_defaultLight.ambient * c_defaultMaterial.ambient;

    // diffuse
    mat3 normalMatrix = mat3(transpose(inverse(u_modelview)));
    vec3 norm = normalize(normalMatrix * normal);
    vec3 lightDir = normalize(-c_defaultLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = c_defaultLight.diffuse * (diff * c_defaultMaterial.diffuse);

    vec3 color = ambient + diffuse;

    float angle = acos(dot(u_printSurfaceNormal, normal) / length(u_printSurfaceNormal) / length(normal));
    bool isOnPrintSurface = (dot(u_printSurfaceNormal, position) + u_printSurfaceD) == 0;

    // нависания
    if (bool(u_mode & 0x04)) {
        if ((angle < u_overhangThreshold) && !(abs(angle - u_overhangThreshold) < epsilon) && !isOnPrintSurface) {
            color.r = 1.0f;
        }
    }

    // слои
    if (bool(u_mode & 0x03) && !(abs(angle) < epsilon) && !(abs(angle - PI) < epsilon)) {
        float dist = (dot(u_printSurfaceNormal, position) + u_printSurfaceD) / length(u_printSurfaceNormal);
        if (abs(dist - (round(dist / u_layerHeight) * u_layerHeight)) < u_lineWidth * abs(sin(angle))) {
            if (bool(u_mode & 0x01)) { // везде
                color.rb = vec2(0.0f, 1.0f);
            } else { // у курсора
                if (length(u_mouseCoord - gl_FragCoord.xy) < u_mouseRadius) {
                    color.rb = vec2(0.0f, 1.0f);
                }
            }
        }
    }

    FragColor = vec4(color, 1.0f);
}

)glsl";

inline const std::string VERTEX_SHADER_CODE_ORIENTATION = R"glsl(
#version 430

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 u_modelview;
uniform mat4 u_projection;

out vec3 normal;

void main() {
    normal = a_normal;

    gl_Position = u_projection * u_modelview * vec4(a_position, 1.0f);
}

)glsl";

inline const std::string FRAGMENT_SHADER_CODE_ORIENTATION = R"glsl(
#version 430

struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
};

const Light c_defaultLight = Light(
    vec3(0.3f, -0.3f, -1.0f),
    vec3(0.2f, 0.2f, 0.2f),
    vec3(0.5f, 0.5f, 0.5f)
);

layout (std430, binding = 0) readonly buffer ssbo_colors {
    vec4 colors[];
};

in vec3 normal;

uniform mat4 u_modelview;

out vec4 FragColor;

void main() {
    float epsilon = 0.01;

    vec3 color = colors[gl_PrimitiveID].rgb;

    // ambient
    vec3 ambient = c_defaultLight.ambient * color;

    // diffuse
    mat3 normalMatrix = mat3(transpose(inverse(u_modelview)));
    vec3 norm = normalize(normalMatrix * normal);
    vec3 lightDir = normalize(-c_defaultLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = c_defaultLight.diffuse * (diff * color);

    FragColor = vec4(ambient + diffuse, 1.0f);
}

)glsl";

inline const std::string POLYLINE_VERT_SHADER_CODE = R"glsl(
#version 330

layout (location = 0) in vec3 a_position;

uniform mat4 u_modelview;
uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * u_modelview * vec4(a_position, 1.0f);
}
)glsl";

inline const std::string POLYLINE_FRAG_SHADER_CODE = R"glsl(
#version 330

uniform vec3 u_color;

out vec4 FragColor;

void main() {
    FragColor = vec4(u_color, 1.0);
}
)glsl";
