///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

struct Light
{
    unsigned int type;
    vec3         color;
    vec3         direction;
    vec3         position;

    vec3         ambient;
    vec3         diffuse;
    vec3         specular;
};

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec2 aBitangent;

// Uniform blocks
layout(binding = 0, std140) uniform GlobalParams
{
    vec3         uCameraPosition;
    unsigned int uLightCount;
    Light        uLight[16];
};

layout(binding = 1, std140) uniform LocalParams
{
    mat4 uWorldMatrix;
    mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition; // In worldspace
out vec3 vNormal;   // In worldspace
out vec3 vViewDir;  // In worldspace

void main()
{
    vTexCoord = aTexCoord;
    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
    vNormal   = mat3(transpose(inverse(uWorldMatrix))) * aNormal; // TODO: Calculate the normal matrix on the CPU and send it to the shaders via a uniform before drawing (just like the model matrix)
    vViewDir = uCameraPosition - vPosition;
    gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

// TODO: Write your fragment shader here
in vec2 vTexCoord;
in vec3 vPosition; // In worldspace
in vec3 vNormal;   // In worldspace
in vec3 vViewDir;  // In worldspace

//uniform sampler2D uTexture;
uniform Material uMaterial;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3         uCameraPosition;
    unsigned int uLightCount;
    Light        uLight[16];
};

layout(location = 0) out vec4 oColor;

void main()
{
    // TODO: Sum all light contributions up to set oColor final value
    vec3 diffuseMap = vec3(texture(uMaterial.diffuse, vTexCoord)); // Texture color
    vec4 specularMap = texture(uMaterial.specular, vTexCoord);
    vec3 lightColor = uLight[0].color;

    // ambient
    vec3 ambient = uLight[0].ambient * diffuseMap;

    // diffuse
    vec3 norm = normalize(vNormal);
    vec3 lightDir;
    switch (uLight[0].type)
    {
        case 0: lightDir = normalize(-uLight[0].direction); break;
        case 1: lightDir = normalize(uLight[0].position - vPosition); break;
        default: break;
    }
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uLight[0].diffuse * diff * diffuseMap;

    // specular
    vec3 viewDir = normalize(vViewDir);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);
    vec3 specular = uLight[0].specular * spec * specularMap.r;

    vec3 result = ambient + diffuse + specular;
    oColor = vec4(result, 1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
