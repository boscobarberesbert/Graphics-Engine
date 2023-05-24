///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef G_BUFFER

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// Uniform blocks
layout(binding = 1, std140) uniform LocalParams
{
    mat4 uWorldMatrix;
    mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition; // In worldspace
out vec3 vNormal;   // In worldspace

void main()
{
    vTexCoord = aTexCoord;
    vPosition = vec3(uWorldMatrix * vec4(aPosition, 1.0));
    vNormal   = mat3(transpose(inverse(uWorldMatrix))) * aNormal; // TODO: Calculate the normal matrix on the CPU and send it to the shaders via a uniform before drawing (just like the model matrix)
    gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
};

// TODO: Write your fragment shader here
layout(location = 0) out vec4 gPosition;   // Fragment positions, depth
layout(location = 1) out vec3 gNormal;     // Normals
layout(location = 2) out vec4 gAlbedoSpec; // Albedo, specular

in vec2 vTexCoord;
in vec3 vPosition; // In worldspace
in vec3 vNormal;   // In worldspace

uniform Material uMaterial;

void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition.rgb = vPosition;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(vNormal);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(uMaterial.diffuse, vTexCoord).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(uMaterial.specular, vTexCoord).r;
    // depth value
    gPosition.a = gl_FragCoord.z;
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
