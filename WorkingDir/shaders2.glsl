///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

struct Light
{
    unsigned int type;
    vec3         color;
    vec3         direction;
    vec3         position; // no longer necessary when using directional lights

    vec3         ambient;
    vec3         diffuse;
    vec3         specular;

    float        constant;
    float        linear;
    float        quadratic;

    float        cutOff;
    float        outerCutOff;
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

vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // TODO: Sum all light contributions up to set oColor final value
    // properties
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(vViewDir);

    vec3 result = vec3(0.0); // define an output color value

    // phase 1: Directional lighting
    //for(int i = 0; i < NR_DIR_LIGHTS; i++) // add the directional light's contribution to the output
        //result += CalcDirLight(dirLights[i], norm, viewDir);
    // phase 2: Point lights
    //for(int i = 0; i < NR_POINT_LIGHTS; i++) // do the same for all point lights
        //result += CalcPointLight(pointLights[i], norm, vPosition, viewDir);
    // phase 3: Spot light
    //for(int i = 0; i < NR_SPOT_LIGHTS; i++) // and add others lights as well (like spotlights)
        //result += CalcSpotLight(spotLights[i], norm, vPosition, viewDir);

    for(int i = 0; i < uLightCount; i++)
    {
        switch (uLight[i].type)
        {
            case 0: result += CalcDirLight(uLight[i], norm, viewDir); break;
            case 1: result += CalcPointLight(uLight[i], norm, vPosition, viewDir); break;
            case 2: result += CalcSpotLight(uLight[i], norm, vPosition, viewDir); break;
            case 3: result += CalcSpotLight(uLight[i], norm, vPosition, viewDir); break;
        }
    }

    oColor = vec4(result, 1.0);
}

vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction); // do directional light calculations
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess == 0 ? 32.0 : uMaterial.shininess);
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(uMaterial.diffuse, vTexCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(uMaterial.diffuse, vTexCoord));
    //vec3 specular = light.specular * spec * vec3(texture(uMaterial.specular, vTexCoord));
    vec3 specular = light.specular * spec * texture(uMaterial.specular, vTexCoord).r;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos); // do light calculations using the light's position
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess == 0 ? 32.0 : uMaterial.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                        light.quadratic * (distance * distance));
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(uMaterial.diffuse, vTexCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(uMaterial.diffuse, vTexCoord));
    //vec3 specular = light.specular * spec * vec3(texture(uMaterial.specular, vTexCoord));
    vec3 specular = light.specular * spec * texture(uMaterial.specular, vTexCoord).r;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos); // do light calculations using the light's position
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess == 0 ? 32.0 : uMaterial.shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                        light.quadratic * (distance * distance));
    // smooth/soft egdes
    float intensity = 1.0; // TODO: Add a light intensity value
    float theta   = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    intensity     = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(uMaterial.diffuse, vTexCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(uMaterial.diffuse, vTexCoord));
    //vec3 specular = light.specular * spec * vec3(texture(uMaterial.specular, vTexCoord));
    vec3 specular = light.specular * spec * texture(uMaterial.specular, vTexCoord).r;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    diffuse  *= intensity;
    specular *= intensity;
    return (ambient + diffuse + specular);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
