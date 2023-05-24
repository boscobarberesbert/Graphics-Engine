///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef DEFERRED_SHADING

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

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

// TODO: Write your fragment shader here
layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform float shininess;

uniform unsigned int renderMode;

layout(binding = 0, std140) uniform GlobalParams
{
    vec3         uCameraPosition;
    unsigned int uLightCount;
    Light        uLight[16];
};

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, vec3 albedo, float specular, float shininess);
vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specular, float shininess);
vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specular, float shininess);

void main()
{
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, vTexCoord).rgb;
    vec3 Normal = texture(gNormal, vTexCoord).rgb;
    vec3 Albedo = texture(gAlbedoSpec, vTexCoord).rgb;
    float Specular = texture(gAlbedoSpec, vTexCoord).a;
    if (Specular == 1.0) Specular = 0.0;
    float depthValue = texture(gPosition, vTexCoord).a;
    float Depth = LinearizeDepth(depthValue) / far;

    // then calculate lighting as usual
    // TODO: Sum all light contributions up to set oColor final value
    // properties
    vec3 viewDir = normalize(uCameraPosition - FragPos);

    vec3 result = vec3(0.0); // define an output color value
    switch (renderMode)
    {
        default:
        case 0:
            for (int i = 0; i < uLightCount; i++)
            {
                switch (uLight[i].type)
                {
                    case 0: result += CalcDirLight(uLight[i], Normal, viewDir, Albedo, Specular, shininess); break;
                    case 1: result += CalcPointLight(uLight[i], Normal, FragPos, viewDir, Albedo, Specular, shininess); break;
                    case 2: result += CalcSpotLight(uLight[i], Normal, FragPos, viewDir, Albedo, Specular, shininess); break;
                    case 3: result += CalcSpotLight(uLight[i], Normal, FragPos, viewDir, Albedo, Specular, shininess); break;
                }
            }
            break;
        case 1: result = Normal; break;
        case 2: result = Albedo; break;
        case 3: result = FragPos; break;
        case 4: result = vec3(Specular); break;
        case 5: result = vec3(Depth); break;
    }

    oColor = vec4(result, 1.0);
}

vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir, vec3 Albedo, float Specular, float shininess)
{
    vec3 lightDir = normalize(-light.direction); // do directional light calculations
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess == 0 ? 32.0 : shininess);
    // combine results
    vec3 ambient  = light.ambient  * Albedo;
    vec3 diffuse  = light.diffuse  * diff * Albedo;
    vec3 specular = light.specular * spec * Specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Albedo, float Specular, float shininess)
{
    vec3 lightDir = normalize(light.position - fragPos); // do light calculations using the light's position
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess == 0 ? 32.0 : shininess);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                        light.quadratic * (distance * distance));
    // combine results
    vec3 ambient  = light.ambient  * Albedo;
    vec3 diffuse  = light.diffuse  * diff * Albedo;
    vec3 specular = light.specular * spec * Specular;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Albedo, float Specular, float shininess)
{
    vec3 lightDir = normalize(light.position - fragPos); // do light calculations using the light's position
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess == 0 ? 32.0 : shininess);
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
    vec3 ambient  = light.ambient  * Albedo;
    vec3 diffuse  = light.diffuse  * diff * Albedo;
    vec3 specular = light.specular * spec * Specular;
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
