#pragma once

#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>  // For GLuint (or unsigned int)

// Function declaration only
GLuint LoadShaders(
    const char* vertex_file_path,
    const char* fragment_file_path
);

#endif // SHADER_HPP