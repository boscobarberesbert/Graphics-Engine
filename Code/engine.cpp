//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include "assimp_model_loading.h"
#include "buffer_management.h"
#include "material.h"

#define BINDING(b) b

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);

    // Fill input vertex shader layout automatically
    GLint attributeCount, attributeNameMaxLength;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeNameMaxLength);
    for (int i = 0; i < attributeCount; ++i)
    {
        char* attributeName = new char[++attributeNameMaxLength];
        GLint attributeNameLength;
        GLint attributeSize;
        GLenum attributeType;
        glGetActiveAttrib(program.handle, i, attributeNameMaxLength, &attributeNameLength, &attributeSize, &attributeType, attributeName);
        u8 attributeLocation = glGetAttribLocation(program.handle, attributeName);

        u8 componentCount = 1;
        switch (attributeType)
        {
        case GL_FLOAT: componentCount = 1; break;
        case GL_FLOAT_VEC2: componentCount = 2; break;
        case GL_FLOAT_VEC3: componentCount = 3; break;
        case GL_FLOAT_VEC4: componentCount = 4; break;
        default: break;
        }

        program.vertexInputLayout.attributes.push_back({ attributeLocation, componentCount });
    }

    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    //stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 1: dataFormat = GL_RED; internalFormat = GL_R8; break;
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    // Try finding a vao for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
        if (submesh.vaos[i].programHandle == program.handle)
            return submesh.vaos[i].handle;

    GLuint vaoHandle = 0;

    // Create a new vao for this submesh/program
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        // We have to link all vertex inputs attributes to attributes in the vertex buffer
        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            bool attributeWasLinked = false;

            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset; // attribute offset + vertex offset
                    const u32 stride = submesh.vertexBufferLayout.stride;
                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }

            assert(attributeWasLinked); // The submesh should provide an attribute for each vertex inputs
        }

        glBindVertexArray(0);
    }

    // Store it in the list of vaos for this submesh
    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

// GL_KHR_debug extension
// GL_KHR_debug - debug callback
void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    ELOG("OpenGL debug message: %s", message);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             ELOG(" - source: GL_DEBUG_SOURCE_API");             break; // Calls to the OpenGL API
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   ELOG(" - source: GL_DEBUG_SOURCE_WINDOW_SYSTEM");   break; // Calls to a window-system API
    case GL_DEBUG_SOURCE_SHADER_COMPILER: ELOG(" - source: GL_DEBUG_SOURCE_SHADER_COMPILER"); break; // A compiler for a shading language
    case GL_DEBUG_SOURCE_THIRD_PARTY:     ELOG(" - source: GL_DEBUG_SOURCE_THIRD_PARTY");     break; // An application associated with OpenGL
    case GL_DEBUG_SOURCE_APPLICATION:     ELOG(" - source: GL_DEBUG_SOURCE_APPLICATION");     break; // Generated by the user of this application
    case GL_DEBUG_SOURCE_OTHER:           ELOG(" - source: GL_DEBUG_SOURCE_OTHER");           break; // Some source that isn't one of these
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               ELOG(" - type: GL_DEBUG_TYPE_ERROR");               break; // An error, typically from the API
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ELOG(" - type: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break; // Some behavior marked deprecated has been used
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ELOG(" - type: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR");  break; // Something has invoked undefined behavior
    case GL_DEBUG_TYPE_PORTABILITY:         ELOG(" - type: GL_DEBUG_TYPE_PORTABILITY");         break; // Some functionality the user relies upon is not portable
    case GL_DEBUG_TYPE_PERFORMANCE:         ELOG(" - type: GL_DEBUG_TYPE_PERFORMANCE");         break; // Code has triggered possible performance issues
    case GL_DEBUG_TYPE_MARKER:              ELOG(" - type: GL_DEBUG_TYPE_MARKER");              break; // Command stream annotation
    case GL_DEBUG_TYPE_PUSH_GROUP:          ELOG(" - type: GL_DEBUG_TYPE_PUSH_GROUP");          break; // Group pushing
    case GL_DEBUG_TYPE_POP_GROUP:           ELOG(" - type: GL_DEBUG_TYPE_POP_GROUP");           break; // Group popping
    case GL_DEBUG_TYPE_OTHER:               ELOG(" - type: GL_DEBUG_TYPE_OTHER");               break; // Some type that isn't one of these
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         ELOG(" - severity: GL_DEBUG_SEVERITY_HIGH");           break; // All OpenGL Errors, shader compilation/linking errors, or highly-dangerous undefined behavior
    case GL_DEBUG_SEVERITY_MEDIUM:       ELOG(" - severity: GL_DEBUG_SEVERITY_MEDIUM");         break; // Major performance warnings, shader compilation/linking warnings, or the use of deprecated functionality
    case GL_DEBUG_SEVERITY_LOW:          ELOG(" - severity: GL_DEBUG_SEVERITY_LOW");            break; // Redundant state change performance warning, or unimportant undefined behavior
    case GL_DEBUG_SEVERITY_NOTIFICATION: ELOG(" - severity: GL_DEBUG_SEVERITY_NOTIFICATION");   break; // Anything that isn't an error or performance issue.
    }
}

mat4 TransformScale(const vec3& scaleFactors)
{
    mat4 transform = scale(scaleFactors);
    return transform;
}

mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors)
{
    mat4 transform = translate(pos);
    transform = scale(transform, scaleFactors);
    return transform;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// Low level GLFW key handling is done in platform.h
void ProcessInput(App* app, GLFWwindow* window)
{
    if (app->input.keys[K_ESCAPE] == BUTTON_PRESSED)
        app->isRunning = false;

    if (app->input.keys[K_W] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(FORWARD, app->deltaTime);
    if (app->input.keys[K_S] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(BACKWARD, app->deltaTime);
    if (app->input.keys[K_A] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(CAM_LEFT, app->deltaTime);
    if (app->input.keys[K_D] == BUTTON_PRESSED)
        app->camera.ProcessKeyboard(CAM_RIGHT, app->deltaTime);
}

// glfw: whenever the mouse moves, this callback is called
void MouseCallback(App* app, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (app->firstMouse) // initially set to true
    {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos; // reversed since y-coordinates go from bottom to top

    app->lastX = xpos;
    app->lastY = ypos;

    app->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void ScrollCallback(App* app, double xoffset, double yoffset)
{
    app->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void CreateEntity(App* app, Entity entity)
{
    app->entities.push_back(entity);
}

void Init(App* app)
{
    // NOT IN USE
    //OpenGLErrorGuard guard("INIT");

    // NOT IN USE
    // During init, enable debug output
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(MessageCallback, 0);

    // GL_KHR_debug extension
    // GL_KHR_debug - debug callback
    if (GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 3))
    {
        glDebugMessageCallback(OnGlError, app);
    }

    // tell GLFW to capture our mouse
    //glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Tell GLFW that it should hide the cursor and capture it

    // configure global opengl state
    glEnable(GL_DEPTH_TEST); // Enable depth testing (Z-buffer), to take into account the fragment depth

    // Retrieve OpenGL information
    app->openglInfo.version = (const char*)glGetString(GL_VERSION);
    app->openglInfo.renderer = (const char*)glGetString(GL_RENDERER);
    app->openglInfo.vendor = (const char*)glGetString(GL_VENDOR);
    app->openglInfo.glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    app->openglInfo.numExtensions = numExtensions;
    for (int i = 0; i < numExtensions; ++i)
    {
        app->openglInfo.extensions.push_back((const char*)glGetStringi(GL_EXTENSIONS, GLuint(i)));
    }

    // TODO: Initialize your resources here!
    // - vertex buffers
    // - element/index buffers
    // - vaos
    // - programs (and retrieve uniform indices)
    // - textures

    app->mode = Mode_TexturedMesh;

    SetupDefaultMaterials(app);

    // Creating uniform buffers
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

    glGenBuffers(1, &app->cbuffer.handle);
    glBindBuffer(GL_UNIFORM_BUFFER, app->cbuffer.handle);
    glBufferData(GL_UNIFORM_BUFFER, app->maxUniformBufferSize, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Camera setup
    app->camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));

    u32 texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    app->programIndexes.insert(std::make_pair("shaders", texturedMeshProgramIdx));
    Program& texturedMeshProgram = app->programs[texturedMeshProgramIdx];
    texturedMeshProgram.programUniformTexture = glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.diffuse");

    u32 specularTexturedMeshProgramIdx = LoadProgram(app, "shaders2.glsl", "TEXTURED_GEOMETRY");
    app->programIndexes.insert(std::make_pair("shaders2", specularTexturedMeshProgramIdx));
    Program& specularTexturedMeshProgram = app->programs[specularTexturedMeshProgramIdx];
    specularTexturedMeshProgram.programUniformTexture = glGetUniformLocation(specularTexturedMeshProgram.handle, "uMaterial.diffuse");
    specularTexturedMeshProgram.programUniformSpecularMap = glGetUniformLocation(specularTexturedMeshProgram.handle, "uMaterial.specular");

    u32 emissiveTexturedMeshProgramIdx = LoadProgram(app, "shaders3.glsl", "TEXTURED_GEOMETRY");
    app->programIndexes.insert(std::make_pair("shaders3", emissiveTexturedMeshProgramIdx));
    Program& emissiveTexturedMeshProgram = app->programs[emissiveTexturedMeshProgramIdx];
    emissiveTexturedMeshProgram.programUniformTexture = glGetUniformLocation(emissiveTexturedMeshProgram.handle, "uMaterial.diffuse");
    emissiveTexturedMeshProgram.programUniformEmissionMap = glGetUniformLocation(emissiveTexturedMeshProgram.handle, "uMaterial.emission");

    u32 lightSourceProgramIdx = LoadProgram(app, "light_source.glsl", "LIGHT_SOURCE");
    app->programIndexes.insert(std::make_pair("light source", lightSourceProgramIdx));

    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

    Material sciFiWallMaterial;
    sciFiWallMaterial.albedoTextureIdx = LoadTexture2D(app, "Materials/Sci-fi_Wall_011_SD/Sci-fi_Wall_011_basecolor.jpg");
    sciFiWallMaterial.emissiveTextureIdx = LoadTexture2D(app, "Materials/Sci-fi_Wall_011_SD/Sci-fi_Wall_011_emissive.jpg");
    sciFiWallMaterial.specular = vec3(1.0f);
    sciFiWallMaterial.shininess = 0.5f * 128.0f;
    app->materials.push_back(sciFiWallMaterial);
    app->materialIndexes.insert(std::make_pair("sci-fi wall", app->materials.size() - 1));

    // Load models
    stbi_set_flip_vertically_on_load(true);
    u32 patrickModelIndex = LoadModel(app, "Patrick/Patrick.obj");
    app->modelIndexes.insert(std::make_pair("patrick", patrickModelIndex));

    stbi_set_flip_vertically_on_load(false);
    u32 backpackModelIndex = LoadModel(app, "backpack/backpack.obj");
    app->modelIndexes.insert(std::make_pair("backpack", backpackModelIndex));

    u32 sphereModelIndex = LoadModel(app, "Primitives/sphere.obj");
    app->modelIndexes.insert(std::make_pair("sphere", sphereModelIndex));

    // Scene setup
    CreateEntity(app, TexturedMesh(app->modelIndexes["patrick"], app->programIndexes["shaders"], vec3(0.0f, 0.0f, 0.0f)));
    CreateEntity(app, TexturedMesh(app->modelIndexes["patrick"], app->programIndexes["shaders"], vec3(-5.0f, 0.0f, -5.0f)));
    CreateEntity(app, TexturedMesh(app->modelIndexes["patrick"], app->programIndexes["shaders"], vec3(5.0f, 0.0f, -5.0f)));
    CreateEntity(app, TexturedMesh(app->modelIndexes["backpack"], app->programIndexes["shaders2"], vec3(0.0f, 0.0f, 2.5f)));
    CreateEntity(app, Primitive(app->materialIndexes["sci-fi wall"], app->modelIndexes["sphere"], app->programIndexes["shaders3"], vec3(2.5f), vec3(0.0f), vec3(0.125f)));

    Light pointLight(LightType_Point, vec3(1.0f), vec3(-5.0f), vec3(5.0f), vec3(0.2f), vec3(0.5f), vec3(1.0f));
    app->lights.push_back(pointLight);

    Light directionalLight(LightType_Directional, vec3(1.0f), vec3(-5.0f), vec3(5.0f), vec3(0.2f), vec3(0.5f), vec3(1.0f));
    app->lights.push_back(directionalLight);

    Light flashLight(LightType_Flash, vec3(1.0f), vec3(-5.0f), vec3(5.0f), vec3(0.2f), vec3(0.5f), vec3(1.0f));
    app->lights.push_back(flashLight);

    CreateEntity(app, LightSource(0, app->modelIndexes["sphere"], app->modelIndexes["light source"], pointLight.position, vec3(0.0f), vec3(0.025f)));
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::End();

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("OpenGL Info"))
        {
            ImGui::Text("OpenGL version: %s", app->openglInfo.version.c_str());
            ImGui::Text("OpenGL renderer: %s", app->openglInfo.renderer.c_str());
            ImGui::Text("OpenGL vendor: %s", app->openglInfo.vendor.c_str());
            ImGui::Text("OpenGL GLSL version: %s", app->openglInfo.glslVersion.c_str());
            const char* buttonText;
            if (!app->openglInfo.showExtensions)
                buttonText = "Show OpenGL extensions";
            else
                buttonText = "Hide OpenGL extensions";
            if (ImGui::Button(buttonText))
                app->openglInfo.showExtensions = !app->openglInfo.showExtensions;
            if (app->openglInfo.showExtensions)
            {
                for (int i = 0; i < app->openglInfo.numExtensions; ++i)
                {
                    ImGui::BulletText(app->openglInfo.extensions.at(i).c_str());
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Update(App* app)
{
    // In Update() -> check timestamp / reload
    // TO DO: Put it in an ImGui button in order not to update it in each frame
    for (u64 i = 0; i < app->programs.size(); ++i)
    {
        Program& program = app->programs[i];
        u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimestamp > program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            String programSource = ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = CreateProgramFromSource(programSource, programName);
            program.lastWriteTimestamp = currentTimestamp;
        }
    }

    // You can handle app->input keyboard/mouse here
    ProcessInput(app, glfwGetCurrentContext());

    // Move the light source around the scene over time
    app->lights[0].position.x = sin(glfwGetTime()) * 5.0f;
    app->lights[0].position.y = sin(glfwGetTime() / 2) * 5.0f;
    app->entities[5].worldMatrix = glm::translate(mat4(1.0f), app->lights[0].position);
    app->entities[5].worldMatrix = glm::scale(app->entities[5].worldMatrix, vec3(0.025f));

    // Change the light's colors over time by changing the light's ambient and diffuse colors
    app->lights[0].color.x = sin(glfwGetTime() * 2.0f);
    app->lights[0].color.y = sin(glfwGetTime() * 0.7f);
    app->lights[0].color.z = sin(glfwGetTime() * 1.3f);

    app->lights[0].diffuse = app->lights[0].color * glm::vec3(0.5f);
    app->lights[0].ambient = app->lights[0].diffuse * glm::vec3(0.2f);

    // View matrix
    mat4 view;
    switch (app->camera.cameraMode)
    {
    case CameraMode_Free:
        view = app->camera.GetViewMatrix();
        break;
    case CameraMode_Orbital:
        {
            const float radius = 10.0f;
            float camX = sin(glfwGetTime()) * radius;
            float camZ = cos(glfwGetTime()) * radius;
            view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
        }
        break;
    default:
        break;
    }

    // Projection matrix
    mat4 projection;
    float aspectRatio = (float)app->displaySize.x / (float)app->displaySize.y;
    float znear = 0.1f;
    float zfar = 100.0f;
    switch (app->camera.cameraProjectionMode)
    {
    case CameraProjectionMode_Orthographic: // Orthographic projection matrix
        projection = glm::ortho(0.0f, (float)app->displaySize.x, 0.0f, (float)app->displaySize.y, znear, zfar);
        break;
    case CameraProjectionMode_Perspective: // Perspective projection matrix
        projection = glm::perspective(glm::radians(app->camera.zoom), aspectRatio, znear, zfar);
        break;
    default:
        break;
    }

    // Filling uniform buffers
    glBindBuffer(GL_UNIFORM_BUFFER, app->cbuffer.handle);
    app->cbuffer.data = (u8*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    app->cbuffer.head = 0;

    // Pushing values for the GlobalParams block into the uniform buffer
    // -- Global params
    app->globalParamsOffset = app->cbuffer.head;

    PushVec3(app->cbuffer, app->camera.position);

    PushUInt(app->cbuffer, app->lights.size());

    for (u32 i = 0; i < app->lights.size(); ++i)
    {
        AlignHead(app->cbuffer, sizeof(vec4));

        Light& light = app->lights[i];
        PushUInt(app->cbuffer, light.type);
        PushVec3(app->cbuffer, light.color);
        if (light.type == LightType_Flash)
        {
            PushVec3(app->cbuffer, app->camera.front);
            PushVec3(app->cbuffer, app->camera.position);
        }
        else
        {
            PushVec3(app->cbuffer, light.direction);
            PushVec3(app->cbuffer, light.position);
        }

        PushVec3(app->cbuffer, light.ambient);
        PushVec3(app->cbuffer, light.diffuse);
        PushVec3(app->cbuffer, light.specular);

        PushFloat(app->cbuffer, light.constant);
        PushFloat(app->cbuffer, light.linear);
        PushFloat(app->cbuffer, light.quadratic);

        PushFloat(app->cbuffer, glm::cos(glm::radians(light.cutOff)));
        PushFloat(app->cbuffer, glm::cos(glm::radians(light.outerCutOff)));
    }

    app->globalParamsSize = app->cbuffer.head - app->globalParamsOffset;

    // Pushing values for the LocalParams block into the uniform buffer 
    // -- Local params
    for (u32 i = 0; i < app->entities.size(); ++i)
    {
        AlignHead(app->cbuffer, app->uniformBlockAlignment);

        Entity& entity = app->entities[i];
        mat4    world = entity.worldMatrix;
        mat4    worldViewProjection = projection * view * world; // note that we read the multiplication from right to left

        entity.localParamsOffset = app->cbuffer.head;
        PushMat4(app->cbuffer, world);
        PushMat4(app->cbuffer, worldViewProjection);
        entity.localParamsSize = app->cbuffer.head - entity.localParamsOffset;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Render(App* app)
{
    // NOT IN USE
    //OpenGLErrorGuard guard("RENDER");

    switch (app->mode)
    {
        case Mode_TexturedQuad:
            {
                // TODO: Draw your textured quad here!
                // - clear the framebuffer
                // - set the viewport
                // - set the blending state
                // - bind the texture into unit 0
                // - bind the program 
                //   (...and make its texture sample from unit 0)
                // - bind the vao
                // - glDrawElements() !!!
                
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glViewport(0, 0, app->displaySize.x, app->displaySize.y);

                Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx];
                glUseProgram(programTexturedGeometry.handle);
                glBindVertexArray(app->vao);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glUniform1i(app->programUniformTexture, 0);
                glActiveTexture(GL_TEXTURE0);
                GLuint textureHandle = app->textures[app->diceTexIdx].handle;
                glBindTexture(GL_TEXTURE_2D, textureHandle);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

                glBindVertexArray(0);
                glUseProgram(0);
            }
            break;
        case Mode_TexturedMesh:
            {
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Bind the buffer range with the global parameters (camera position, lights...) to the GlobalParams block in the shader
                glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, app->globalParamsOffset, app->globalParamsSize);

                for (u16 i = 0; i < app->entities.size(); ++i)
                {
                    Entity entity = app->entities.at(i);

                    // Binding buffer ranges to uniform blocks
                    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->cbuffer.handle, entity.localParamsOffset, entity.localParamsSize);

                    /*Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];*/
                    Program texturedMeshProgram;
                    switch (entity.type)
                    {
                    case EntityType_Primitive:   texturedMeshProgram = app->programs[entity.programIndex];                 break;
                    case EntityType_Model:       texturedMeshProgram = app->programs[entity.programIndex];                 break;
                    case EntityType_LightSource: texturedMeshProgram = app->programs[app->programIndexes["light source"]]; break;
                    default: break;
                    }
                    glUseProgram(texturedMeshProgram.handle);

                    Model& model = app->models[entity.modelIndex];
                    Mesh& mesh = app->meshes[model.meshIdx];

                    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                    {
                        GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                        glBindVertexArray(vao);

                        u32 submeshMaterialIdx = model.materialIdx[i];
                        Material& submeshMaterial = app->materials[submeshMaterialIdx];

                        switch (entity.type)
                        {
                        case EntityType_Primitive:
                            {
                                Material material = app->materials[entity.materialIndex];
                                if (texturedMeshProgram.handle == app->programs[app->programIndexes["shaders"]].handle)
                                {
                                    glActiveTexture(GL_TEXTURE0);
                                    glBindTexture(GL_TEXTURE_2D, app->textures[material.albedoTextureIdx].handle);
                                    glUniform1i(texturedMeshProgram.programUniformTexture, 0);

                                    glUniform3fv(glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.specular"), 1, glm::value_ptr(material.specular));
                                    glUniform1f(glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.shininess"), material.shininess);
                                }
                                else if (texturedMeshProgram.handle == app->programs[app->programIndexes["shaders2"]].handle)
                                {
                                    glActiveTexture(GL_TEXTURE0);
                                    glBindTexture(GL_TEXTURE_2D, app->textures[material.albedoTextureIdx].handle);
                                    glUniform1i(texturedMeshProgram.programUniformTexture, 0);

                                    glActiveTexture(GL_TEXTURE1);
                                    glBindTexture(GL_TEXTURE_2D, app->textures[material.specularTextureIdx].handle);
                                    glUniform1i(texturedMeshProgram.programUniformSpecularMap, 1);

                                    glUniform1f(glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.shininess"), material.shininess);
                                }
                                if (texturedMeshProgram.handle == app->programs[app->programIndexes["shaders3"]].handle)
                                {
                                    glActiveTexture(GL_TEXTURE0);
                                    glBindTexture(GL_TEXTURE_2D, app->textures[material.albedoTextureIdx].handle);
                                    glUniform1i(texturedMeshProgram.programUniformTexture, 0);

                                    glUniform3fv(glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.specular"), 1, glm::value_ptr(material.specular));
                                    glUniform1f(glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.shininess"), material.shininess);

                                    glActiveTexture(GL_TEXTURE1);
                                    glBindTexture(GL_TEXTURE_2D, app->textures[material.emissiveTextureIdx].handle);
                                    glUniform1i(texturedMeshProgram.programUniformEmissionMap, 1);
                                }
                            }
                            break;
                        case EntityType_Model:
                            {
                                if (texturedMeshProgram.handle == app->programs[app->programIndexes["shaders"]].handle)
                                {
                                    glActiveTexture(GL_TEXTURE0);
                                    glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                                    glUniform1i(texturedMeshProgram.programUniformTexture, 0);

                                    glUniform3fv(glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.specular"), 1, glm::value_ptr(submeshMaterial.specular));
                                    glUniform1f(glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.shininess"), submeshMaterial.shininess);
                                }
                                else if (texturedMeshProgram.handle == app->programs[app->programIndexes["shaders2"]].handle)
                                {
                                    glActiveTexture(GL_TEXTURE0);
                                    glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                                    glUniform1i(texturedMeshProgram.programUniformTexture, 0);

                                    glActiveTexture(GL_TEXTURE1);
                                    glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.specularTextureIdx].handle);
                                    glUniform1i(texturedMeshProgram.programUniformSpecularMap, 1);

                                    glUniform1f(glGetUniformLocation(texturedMeshProgram.handle, "uMaterial.shininess"), submeshMaterial.shininess);
                                }
                            }
                            break;
                        default:
                            break;
                        }

                        Submesh& submesh = mesh.submeshes[i];
                        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                    }
                }
            }
            break;

        default:;
    }
}

// NOT IN USE
void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar* message,
                const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}
