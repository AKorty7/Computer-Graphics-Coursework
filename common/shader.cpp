#include "shader.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {
    // 1. Create shaders
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // 2. Load vertex shader code
    std::string vertex_shader_code;
    std::ifstream vertex_shader_stream(vertex_file_path, std::ios::in);
    if (vertex_shader_stream.is_open()) {
        std::stringstream sstr;
        sstr << vertex_shader_stream.rdbuf();
        vertex_shader_code = sstr.str();
        vertex_shader_stream.close();
    }
    else {
        std::cerr << "Error: Failed to open vertex shader file: " << vertex_file_path << std::endl;
        return 0;
    }

    // 3. Load fragment shader code
    std::string fragment_shader_code;
    std::ifstream fragment_shader_stream(fragment_file_path, std::ios::in);
    if (fragment_shader_stream.is_open()) {
        std::stringstream sstr;
        sstr << fragment_shader_stream.rdbuf();
        fragment_shader_code = sstr.str();
        fragment_shader_stream.close();
    }
    else {
        std::cerr << "Error: Failed to open fragment shader file: " << fragment_file_path << std::endl;
        return 0;
    }

    // 4. Compile vertex shader
    std::cout << "Compiling vertex shader: " << vertex_file_path << std::endl;
    const char* vertex_source_ptr = vertex_shader_code.c_str();
    glShaderSource(vertex_shader_id, 1, &vertex_source_ptr, nullptr);
    glCompileShader(vertex_shader_id);

    // Check for errors
    GLint success = GL_FALSE;
    int info_log_length;
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> error_message(info_log_length + 1);
        glGetShaderInfoLog(vertex_shader_id, info_log_length, nullptr, &error_message[0]);
        std::cerr << "Vertex shader error:\n" << &error_message[0] << std::endl;
    }

    // 5. Compile fragment shader (same process)
    std::cout << "Compiling fragment shader: " << fragment_file_path << std::endl;
    const char* fragment_source_ptr = fragment_shader_code.c_str();
    glShaderSource(fragment_shader_id, 1, &fragment_source_ptr, nullptr);
    glCompileShader(fragment_shader_id);

    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
    glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> error_message(info_log_length + 1);
        glGetShaderInfoLog(fragment_shader_id, info_log_length, nullptr, &error_message[0]);
        std::cerr << "Fragment shader error:\n" << &error_message[0] << std::endl;
    }

    // 6. Link the program
    std::cout << "Linking shader program..." << std::endl;
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    // Check linking errors
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        std::vector<char> error_message(info_log_length + 1);
        glGetProgramInfoLog(program_id, info_log_length, nullptr, &error_message[0]);
        std::cerr << "Shader program linking error:\n" << &error_message[0] << std::endl;
    }

    // 7. Cleanup shaders (they're now linked into the program)
    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return program_id;
}