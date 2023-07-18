#ifndef SHADERS_HPP
#define SHADERS_HPP

inline const char* VERTEX_SHADER_CODE = R"glsl(
#version 330

layout (location = 0) in vec3 a_pos;

uniform mat4 u_modelview;
uniform mat4 u_projection;

void main() {
   gl_Position = u_projection * u_modelview * vec4(a_pos, 1.0f);
}

)glsl";

inline const char* FRAGMENT_SHADER_CODE = R"glsl(
#version 330

out vec4 FragColor;

void main() {
    FragColor = vec4(0.0f, 0.0f, 1.0f, 0.3f);
}

)glsl";

#endif /* SHADERS_HPP */
