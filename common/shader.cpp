#include "shader.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace ShaderUtils
{
    std::string loadShaderCode(const char* file_path)
    {
        std::string shader_code;
        std::ifstream shader_stream(file_path, std::ios::in);
        if (shader_stream.is_open()) {
            std::stringstream sstr;
            sstr << shader_stream.rdbuf();
            shader_code = sstr.str();
            shader_stream.close();
        }
        else {
            std::cerr << "Error: Failed to open shader file: " << file_path << std::endl;
        }
        return shader_code;
    }

    GLuint compileShader(GLuint shader_id, const std::string& shader_code, const char* file_path)
    {
        std::cout << "Compiling shader: " << file_path << std::endl;
        const char* source_ptr = shader_code.c_str();
        glShaderSource(shader_id, 1, &source_ptr, nullptr);
        glCompileShader(shader_id);

        // Check for errors
        GLint success = GL_FALSE;
        int info_log_length;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
        if (info_log_length > 0) {
            std::vector<char> error_message(info_log_length + 1);
            glGetShaderInfoLog(shader_id, info_log_length, nullptr, &error_message[0]);
            std::cerr << "Shader error:\n" << &error_message[0] << std::endl;
        }
        return shader_id;
    }

    GLuint linkProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
    {
        std::cout << "Linking shader program..." << std::endl;
        GLuint program_id = glCreateProgram();
        glAttachShader(program_id, vertex_shader_id);
        glAttachShader(program_id, fragment_shader_id);
        glLinkProgram(program_id);

        // Check linking errors
        GLint success = GL_FALSE;
        int info_log_length;
        glGetProgramiv(program_id, GL_LINK_STATUS, &success);
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
        if (info_log_length > 0) {
            std::vector<char> error_message(info_log_length + 1);
            glGetProgramInfoLog(program_id, info_log_length, nullptr, &error_message[0]);
            std::cerr << "Shader program linking error:\n" << &error_message[0] << std::endl;
        }
        return program_id;
    }
}

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {
    // 1. Create shaders
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // 2. Load vertex shader code
    std::string vertex_shader_code = ShaderUtils::loadShaderCode(vertex_file_path);
    if (vertex_shader_code.empty()) return 0;

    // 3. Load fragment shader code
    std::string fragment_shader_code = ShaderUtils::loadShaderCode(fragment_file_path);
    if (fragment_shader_code.empty()) return 0;

    // 4. Compile vertex shader
    vertex_shader_id = ShaderUtils::compileShader(vertex_shader_id, vertex_shader_code, vertex_file_path);

    // 5. Compile fragment shader (same process)
    fragment_shader_id = ShaderUtils::compileShader(fragment_shader_id, fragment_shader_code, fragment_file_path);

    // 6. Link the program
    GLuint program_id = ShaderUtils::linkProgram(vertex_shader_id, fragment_shader_id);

    // 7. Cleanup shaders (they're now linked into the program)
    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}