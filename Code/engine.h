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

    GLuint programUniformTexture;     // Location of the texture uniform in the shader
    GLuint programUniformSpecularMap; // Location of the specular map uniform in the shader
    GLuint programUniformEmissionMap; // Location of the emission map uniform in the shader
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Count,
    Mode_TexturedMesh,
    Mode_Deferred
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
    Material() {}

    Material(vec3 ambient, vec3 diffuse, vec3 specular, f32 shininess)
    {
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
        this->shininess = shininess * 128.0f;
    }

    std::string name;
    vec3        albedo;
    vec3        emissive;
    f32         smoothness;
    u32         albedoTextureIdx;
    u32         emissiveTextureIdx;
    u32         specularTextureIdx;
    u32         normalsTextureIdx;
    u32         bumpTextureIdx;

    vec3        ambient;
    vec3        diffuse;
    vec3        specular;
    f32         shininess;
};

enum EntityType
{
    EntityType_Primitive,
    EntityType_Model,
    EntityType_LightSource
};

struct Entity
{
    Entity(vec3 position = vec3(0.0f), vec3 rotation = vec3(0.0f), vec3 scale = vec3(1.0f))
    {
        // TODO: Set default model and program

        this->SetTransform(position, rotation, scale);
    }

    Entity(u32 modelIndex, u32 programIndex, vec3 position = vec3(0.0f), vec3 rotation = vec3(0.0f), vec3 scale = vec3(1.0f))
    {
        this->modelIndex = modelIndex;
        this->programIndex = programIndex;

        this->SetTransform(position, rotation, scale);
    }

    void SetTransform(vec3 position, vec3 rotation, vec3 scale)
    {
        this->worldMatrix = mat4(1.0f);

        this->SetPosition(position);
        this->SetRotation(rotation);
        this->SetScale(scale);
    }

    inline void SetPosition(vec3 position) { this->worldMatrix[3] = glm::vec4(position, 1.0f); } // Set the new position of the matrix without changing its scale or rotation
    void SetRotation(vec3 rotation)
    {
        SetAxisRotation(rotation.x, vec3(1.0f, 0.0f, 0.0f));
        SetAxisRotation(rotation.y, vec3(0.0f, 1.0f, 0.0f));
        SetAxisRotation(rotation.z, vec3(0.0f, 0.0f, 1.0f));
    }
    void SetAxisRotation(f32 angle, vec3 axis)
    {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);

        // Get the original scale and translation of the matrix
        glm::vec3 scale = glm::vec3(glm::length(this->worldMatrix[0]), glm::length(this->worldMatrix[1]), glm::length(this->worldMatrix[2]));
        glm::vec3 translation = glm::vec3(this->worldMatrix[3]);

        // Combine the rotation matrix with the original scale and translation
        this->worldMatrix = glm::mat4(glm::mat3(this->worldMatrix) * glm::mat3(rotationMatrix));
        this->worldMatrix[0] *= scale[0];
        this->worldMatrix[1] *= scale[1];
        this->worldMatrix[2] *= scale[2];
        this->worldMatrix[3] = glm::vec4(translation, 1.0f);
    }
    void SetScale(vec3 scale)
    {
        // Get the original rotation and translation of the matrix
        glm::mat3 rotation = glm::mat3(this->worldMatrix);
        glm::vec3 translation = glm::vec3(this->worldMatrix[3]);

        // Create a new matrix with the new scale and the original rotation and translation
        this->worldMatrix = glm::mat4(rotation);
        this->worldMatrix[0] *= scale[0];
        this->worldMatrix[1] *= scale[1];
        this->worldMatrix[2] *= scale[2];
        this->worldMatrix[3] = glm::vec4(translation, 1.0f);
    }

    inline void Translate(vec3 translate) { this->worldMatrix = glm::translate(mat4(1.0f), translate) * this->worldMatrix; }
    void Rotate(vec3 rotate)
    {
        this->RotateAxis(rotate.x, vec3(1.0f, 0.0f, 0.0f));
        this->RotateAxis(rotate.y, vec3(0.0f, 1.0f, 0.0f));
        this->RotateAxis(rotate.z, vec3(0.0f, 0.0f, 1.0f));
    }
    inline void RotateAxis(f32 angle, vec3 axis) { this->worldMatrix = glm::rotate(angle, axis) * this->worldMatrix; }
    inline void Scale(vec3 scale) { this->worldMatrix = glm::scale(mat4(1.0f), scale) * worldMatrix; }

    glm::mat4  worldMatrix;
    u32        modelIndex;
    u32        localParamsOffset;
    u32        localParamsSize;

    u32        programIndex;
    u32        materialIndex;

    EntityType type;
};

struct Primitive : Entity
{
    Primitive(u32 materialIndex, u32 modelIndex, u32 programIndex, vec3 position = vec3(0.0f), vec3 rotation = vec3(0.0f), vec3 scale = vec3(1.0f)) :
        Entity(modelIndex, programIndex, position, rotation, scale)
    {
        this->materialIndex = materialIndex;

        type = EntityType_Primitive;
    }
};

struct TexturedMesh : Entity
{
    TexturedMesh(u32 modelIndex, u32 programIndex, vec3 position = vec3(0.0f), vec3 rotation = vec3(0.0f), vec3 scale = vec3(1.0f)) :
        Entity(modelIndex, programIndex, position, rotation, scale)
    {
        type = EntityType_Model;
    }
};

struct LightSource : Entity
{
    LightSource(u32 lightIndex, u32 modelIndex, u32 programIndex, vec3 position, vec3 rotation, vec3 scale) : Entity(modelIndex, programIndex, position, rotation, scale)
    {
        this->lightIndex = lightIndex;

        type = EntityType_LightSource;
    }

    u32 lightIndex;
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
    LightType_Point,
    LightType_Spot,
    LightType_Flash
};

struct Light
{
    Light(LightType type = LightType_Directional, vec3 color = vec3(1.0f), vec3 direction = vec3(0.0f, 0.0f, -1.0f), vec3 position = vec3(0.0f),
        vec3 ambient = vec3(0.2f), vec3 diffuse = vec3(0.5f), vec3 specular = vec3(1.0f),
        float constant = 1.0f, float linear = 0.045f, float quadratic = 0.0075f,
        float cutOff = 12.5f, float outerCutOff = 17.5f)
    {
        this->type      = type;
        this->color     = color;
        this->direction = direction;
        this->position  = position;

        this->ambient   = ambient;
        this->diffuse   = diffuse;
        this->specular  = specular;

        this->constant  = constant;
        this->linear    = linear;
        this->quadratic = quadratic;

        this->cutOff = cutOff;
        this->outerCutOff = outerCutOff;
    }

    LightType type;
    vec3      color;
    vec3      direction;
    vec3      position;

    vec3      ambient;
    vec3      diffuse;
    vec3      specular;

    float     constant;
    float     linear;
    float     quadratic;

    float     cutOff;
    float     outerCutOff;
};

struct Framebuffer
{
    unsigned int gPosition;
    unsigned int gNormal;
    unsigned int gAlbedoSpec;
    unsigned int gDepth;
};

enum RenderMode
{
    RenderMode_FinalRender,
    RenderMode_Normals,
    //RenderMode_AmbientOcclusion,
    RenderMode_Albedo,
    RenderMode_Positions,
    RenderMode_Specular
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
    //u32 texturedMeshProgramIdx;
    
    // texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    // Mode
    Mode mode;

    // Render Mode
    RenderMode renderMode;

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

    // Model indexes
    std::map<std::string, u32> modelIndexes;

    // Program indexes
    std::map<std::string, u32> programIndexes;

    // Material indexes
    std::map<std::string, u32> materialIndexes;

    // Uniform buffer memory management
    GLint maxUniformBufferSize;
    GLint uniformBlockAlignment;

    Buffer cbuffer;
    Buffer gBuffer;

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

    // Framebuffer object handles
    Framebuffer framebufferHandles;
};

void Init(App* app);

void Gui(App* app);

void Update(App* app);

void Render(App* app);

u32 LoadTexture2D(App* app, const char* filepath);

void FramebufferSizeCallback(App* app, GLFWwindow* window, int width, int height); // Window resize

//void ProcessInput(App* app, GLFWwindow* window);                                 // Keyboard Input

void MouseCallback(App* app, double xpos, double ypos);                            // Mouse Input (Move - Drag)

void ScrollCallback(App* app, double xoffset, double yoffset);                     // Mouse Input (Scroll - Wheel)
