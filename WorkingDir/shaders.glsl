///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

// TODO: Write your vertex shader here
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec2 aBitangent;

// Uniform blocks
layout(binding = 1, std140) uniform LocalParams
{
    mat4 model;
    mat4 view;
    mat4 projection;
};

out vec2 vTexCoord;
out vec3 vPosition; // In worldspace
out vec3 vNormal;   // In worldspace
out vec3 vViewDir;

void main()
{
    vTexCoord = aTexCoord;

    // We will usually not define the clipping scale manually...
    // it is usually computed by the projection matrix. Because
    // we are not passing uniform transforms yet, we increase
    // the clipping scale so that Patrick fits the screen.
    //float clippingScale = 5.0;

    //gl_Position = vec4(aPosition, clippingScale);

    // Patrick looks away from the camera by default, so I flip it here.
    //gl_Position.z = -gl_Position.z;

    vPosition = vec3(model * vec4(aPosition, 1.0));
    vNormal   = vec3(model * vec4(aNormal, 0.0));

    // note that we read the multiplication from right to left
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

// TODO: Write your fragment shader here
in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
    oColor = texture(uTexture, vTexCoord);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
