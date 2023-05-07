//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>

#include "GLFW/glfw3.h"
#include "camera.h"

#include <map>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;

typedef glm::mat4 mat4;

struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};

struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute> attributes;
    u8                                 stride;
};

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};

struct Vao
{
    GLuint handle;
    GLuint programHandle;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?

    VertexShaderLayout vertexInputLayout;
};

enum Mode
{
    Mode_TexturedMesh,
    Mode_TexturedQuad,
    Mode_Count
};

struct OpenGLInfo
{
    std::string version;
    std::string renderer;
    std::string vendor;
    std::string glslVersion;
    u32 numExtensions;
    std::vector<std::string> extensions;
    bool showExtensions;
};

struct VertexV3V2
{
    glm::vec3 pos;
    glm::vec2 uv;
};

struct Model
{
    u32              meshIdx;
    std::vector<u32> materialIdx;
};

struct Submesh
{
    VertexBufferLayout vertexBufferLayout;
    std::vector<float> vertices;
    std::vector<u32>   indices;
    u32                vertexOffset;
    u32                indexOffset;

    std::vector<Vao>   vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint               vertexBufferHandle;
    GLuint               indexBufferHandle;
};

struct Material
{
    std::string name;
    vec3        albedo;
    vec3        emissive;
    f32         smoothness;
    u32         albedoTextureIdx;
    u32         emissiveTextureIdx;
    u32         specularTextureIdx;
    u32         normalsTextureIdx;
    u32         bumpTextureIdx;
};

enum EntityType
{
    EntityType_TexturedGeometry,
    EntityType_TexturedMesh,
    EntityType_LightSource
};

struct Entity
{
    glm::mat4 worldMatrix;
    u32       modelIndex;
    u32       localParamsOffset;
    u32       localParamsSize;

    EntityType type;
};

struct Buffer
{
    GLuint handle;
    GLenum type;
    u32    size;
    u32    head;
    void* data; // mapped data
};

enum LightType
{
    LightType_Directional,
    LightType_Point
};

struct Light
{
    LightType type;
    vec3      color;
    vec3      direction;
    vec3      position;
};

struct LightSource : Entity
{
    u32 lightIndex;
};

struct App
{
    // Loop
    f32  deltaTime;
    bool isRunning;

    // Input
    Input input;

    // Graphics
    char gpuName[64];
    char openGlVersion[64];

    ivec2 displaySize;

    // Resources
    std::vector<Texture>  textures;
    std::vector<Material> materials;
    std::vector<Mesh>     meshes;
    std::vector<Model>    models;
    std::vector<Program>  programs;

    // program indices
    u32 texturedGeometryProgramIdx;
    u32 texturedMeshProgramIdx;
    u32 lightSourceProgramIdx;
    
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    // Mode
    Mode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    GLuint embeddedVertices;
    GLuint embeddedElements;

    // Location of the texture uniform in the textured quad shader
    GLuint programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    GLuint vao;

    // OpenGL information
    OpenGLInfo openglInfo;

    // Model
    std::map<std::string, u32> modelIndexes;

    // Uniform buffer memory management
    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;

    Buffer cbuffer;

    // Global params
    u32 globalParamsOffset;
    u32 globalParamsSize;

    // Camera
    Camera camera;

    // Last mouse positions (initialized in the center of the screen)
    float lastX = displaySize.x / 2.0f;
    float lastY = displaySize.y / 2.0f;

    // To check if it's the first time we receive mouse input
    bool firstMouse = true;

    // List of entities
    std::vector<Entity> entities;

    // List of lights
    std::vector<Light> lights;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

u32 LoadTexture2D(App* app, const char* filepath);

void FramebufferSizeCallback(GLFWwindow* window, int width, int height); // Window resize

//void ProcessInput(App* app, GLFWwindow* window);                       // Keyboard Input

void MouseCallback(App* app, double xpos, double ypos);                  // Mouse Input (Move - Drag)

void ScrollCallback(App* app, double xoffset, double yoffset);           // Mouse Input (Scroll - Wheel)
