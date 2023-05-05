#pragma once

#ifndef OPENGL_ERROR_GUARD_H
#define OPENGL_ERROR_GUARD_H

class OpenGLErrorGuard
{
public:

    OpenGLErrorGuard(const char* message) : msg(message) {
        CheckGLError("BEGIN", msg);
    }

    ~OpenGLErrorGuard() {
        CheckGLError("END", msg);
    }

    static void CheckGLError(const char* around, const char* message);

    const char* msg;
};

#endif // OPENGL_ERROR_GUARD_H