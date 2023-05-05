#include "opengl_error_guard.h"
#include "platform.h"
#include <glad/glad.h>

void OpenGLErrorGuard::CheckGLError(const char* around, const char* message)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        // Process/log the error.
        std::string errStr = "GL Error: ";
        switch (err)
        {
        case GL_INVALID_ENUM: errStr += "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE: errStr += "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: errStr += "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW: errStr += "GL_STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW: errStr += "GL_STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY: errStr += "GL_OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errStr += "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        default: break;
        }
        errStr += (" around %s of %s.", around, message);
        LogString(errStr.c_str());
    }
}