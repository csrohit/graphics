#include "renderer.h"

#include <OpenGL/gl3.h>
#include <cstdlib>
#include <cstring>
#include "vmath.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace vmath;
#define INCREMENT_ANGLE 0.1f;

GLint  compileShader(unsigned int shaderId, const char *shaderSourceCode);
GLint  linkProgram(GLuint programId);
GLuint loadShaders(const char *vertexShaderSourceCode, const char *fragmentShaderSourceCode);
GLuint loadCubemap(char **faces);
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD
};

const GLchar *vertexShaderSource = "#version 410 core"
                                   "\n"
                                   "in vec4 aPosition;"
                                   "in vec4 aColor;"
                                   "out vec4 oColor;"
                                   "uniform mat4 uMVPMatrix;"
                                   "void main(void)"
                                   "{"
                                   "gl_Position =uMVPMatrix * aPosition;"
                                   "oColor = aColor;"
                                   "}";

const GLchar *fragmentShaderSource = "#version 410 core"
                                     "\n"
                                     "in vec4 oColor;"
                                     "out vec4 FragColor;"
                                     "void main(void)"
                                     "{"
                                     "FragColor=oColor;"
                                     "}";

const GLchar *skyoxVertexShaderSourceCode = "#version 410 core"
                                            "\n"
                                            "in vec4 aPosition;"
                                            "out vec3 texCoords;"
                                            "uniform mat4 uMVPMatrix;"
                                            "void main(void)"
                                            "{"
                                            "   vec4 pos = uMVPMatrix * aPosition;"
                                            "   gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);"
                                            "   texCoords   = vec3(aPosition.x, aPosition.y, -aPosition.z);"
                                            "}";

const GLchar *skyboxFragmentShaderSourceCode = "#version 410 core"
                                               "\n"
                                               "out vec4 FragColor;"
                                               "\n"
                                               "in vec3 texCoords;"
                                               "\n"
                                               "uniform samplerCube skybox;"
                                               "\n"
                                               "void main()"
                                               "{"
                                               "    FragColor = texture(skybox, texCoords);"
                                               "}";

GLuint shaderProgramObject   = 0U;
GLuint skyboxShaderProgramId = 0U;

/* Data */

const GLfloat modelPositions[] = {
    /* Front */
    1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,

    /* Top */
    1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,

    /* Bottom */
    1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,

    /* Back */
    1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,

    /* Right */
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    -1.0f,

    /* Left */
    -1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    -1.0f,
    1.0f,
};

float skyboxVertices[] = {-1.0f, -1.0f, 1.0f,  //        7--------6
                          1.0f,  -1.0f, 1.0f,  //       /|       /|
                          1.0f,  -1.0f, -1.0f, //      4--------5 |
                          -1.0f, -1.0f, -1.0f, //      | |      | |
                          -1.0f, 1.0f,  1.0f,  //      | 3------|-2
                          1.0f,  1.0f,  1.0f,  //      |/       |/
                          1.0f,  1.0f,  -1.0f, //      0--------1
                          -1.0f, 1.0f,  -1.0f};

unsigned int skyboxIndices[] = {
    // Right
    1, 2, 6, 6, 5, 1,
    // Left
    0, 4, 7, 7, 3, 0,
    // Top
    4, 5, 6, 6, 7, 4,
    // Bottom
    0, 3, 2, 2, 1, 0,
    // Back
    0, 1, 5, 5, 4, 0,
    // Front
    3, 7, 6, 6, 2, 3};

GLuint vaoModel          = 0U;
GLuint vboModelPositions = 0U;

GLuint vaoSkybox          = 0U;
GLuint vboSkyboxPositions = 0U;
GLuint eboSkyboxIndices   = 0U;

/* Uniforms */
GLuint mvpMatrixUniform       = 0U;
GLuint skyboxMVPMatrixUniform = 0U;
GLuint skyboxSamplerUniform   = 0U;

/* Uniform data binding */
mat4   projectionMatrix = {0};
GLuint textureSkybox    = 0U;

/* Functional variables */
GLfloat     rotationAngle = 0.0f;
const char *arr[]         = {"./textures/right.jpg", "./textures/left.jpg", "./textures/top.jpg", "./textures/bottom.jpg", "./textures/front.jpg", "./textures/back.jpg"};

vmath::vec3 eyePosition = vmath::vec3(0.0f, 0.0f, 2.0f);
vmath::vec3 eyeTarget   = vmath::vec3(0.0f, 0.0f, 0.0f);

int initialize()
{

    shaderProgramObject = loadShaders(vertexShaderSource, fragmentShaderSource);
    if (0U == shaderProgramObject)
    {
        fprintf(gpFile, "Failed to load shaders \n");
        return -1;
    }
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "aColor");
    if (GL_FALSE == linkProgram(shaderProgramObject))
    {
        fprintf(gpFile, "Failed to link shaders\n");
        glDeleteProgram(shaderProgramObject);
        return -1;
    }
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");

    skyboxShaderProgramId = loadShaders(skyoxVertexShaderSourceCode, skyboxFragmentShaderSourceCode);
    if (0U == shaderProgramObject)
    {
        fprintf(gpFile, "Failed to load shaders \n");
        return -1;
    }
    glBindAttribLocation(skyboxShaderProgramId, AMC_ATTRIBUTE_POSITION, "aPosition");
    if (GL_FALSE == linkProgram(skyboxShaderProgramId))
    {
        fprintf(gpFile, "Failed to link shaders\n");
        glDeleteProgram(skyboxShaderProgramId);
        return -1;
    }
    skyboxMVPMatrixUniform = glGetUniformLocation(skyboxShaderProgramId, "uMVPMatrix");
    skyboxSamplerUniform   = glGetUniformLocation(skyboxShaderProgramId, "skybox");

    /* Sqaure */
    glGenVertexArrays(1, &vaoModel);
    glBindVertexArray(vaoModel);
    glGenBuffers(1, &vboModelPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboModelPositions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(modelPositions), modelPositions, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
    glBindVertexArray(0U);

    /* Skybox */
    glGenVertexArrays(1, &vaoSkybox);
    glBindVertexArray(vaoSkybox);
    glGenBuffers(1, &vboSkyboxPositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboSkyboxPositions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    glGenBuffers(1, &eboSkyboxIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSkyboxIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);
    glBindVertexArray(0U);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    textureSkybox = loadCubemap((char **)arr);

    projectionMatrix = mat4::identity();

    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST); //[Compulsory] enable depth test
    glDepthFunc(GL_LESS);    //[Compulsory] Which function to use for testing
    return (0);
}

void onKeyDown(int key)
{
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window with color whose bit is set, all bits in depth buffer set to 1 (if value is 1 or lower because of LEQUAL)

    vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
    vmath::mat4 translationMatrix         = vmath::mat4::identity();
    vmath::mat4 scaleMatrix               = vmath::mat4::identity();
    vmath::mat4 viewMatrix                = vmath::mat4::identity();
    vmath::mat4 rotationMatrix            = vmath::mat4::identity();

    /* cube */
    glUseProgram(shaderProgramObject);
    glBindVertexArray(vaoModel);
    {
        translationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
        rotationMatrix    = vmath::rotate(rotationAngle, 0.0f, 1.0f, 0.0f);
        viewMatrix        = vmath::lookat(eyePosition, eyeTarget, vmath::vec3(0.0f, 1.0f, 0.0f));

        modelViewProjectionMatrix = projectionMatrix * viewMatrix * translationMatrix * rotationMatrix * scaleMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
        glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
    }
    glBindVertexArray(0U);
    glUseProgram(0);

    /* Skybox */
    glUseProgram(skyboxShaderProgramId);
    glBindVertexArray(vaoSkybox);
    {
        rotationMatrix    = vmath::rotate(rotationAngle, 0.0f, 1.0f, 0.0f);
        scaleMatrix       = vmath::scale(13.0f, 13.0f, 13.0f);
        viewMatrix        = vmath::lookat(eyePosition, eyeTarget, vmath::vec3(0.0f, 1.0f, 0.0f));
        translationMatrix = vmath::mat4::identity();

        modelViewProjectionMatrix = projectionMatrix * viewMatrix * rotationMatrix * scaleMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureSkybox);
        glUniform1i(skyboxSamplerUniform, 0);

        glDepthFunc(GL_LEQUAL);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LESS);
    }
    glBindVertexArray(0U);
    glUseProgram(0);
}

void update()
{
    rotationAngle = rotationAngle + INCREMENT_ANGLE;
    if (360.0f < rotationAngle)
    {
        rotationAngle = rotationAngle - 360.0f;
    }
}

void resize(int width, int height)
{
    if (height <= 0)
        height = 1;
    /* @note: while writing this code the following issue is observed hence
       multiplying by 2
       https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
    */
    glViewport(0, 0, (GLsizei)width * 2, (GLsizei)height * 2);

    projectionMatrix = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void uninitialize()
{

    if (0U != shaderProgramObject)
    {
        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0U;
    }
    if (0U != skyboxShaderProgramId)
    {
        glDeleteProgram(skyboxShaderProgramId);
        skyboxShaderProgramId = 0U;
    }

    if (0U != vboModelPositions)
    {
        glDeleteBuffers(1, &vboModelPositions);
        vboModelPositions = 0U;
    }

    if (0U != vaoModel)
    {
        glDeleteVertexArrays(1, &vaoModel);
        vaoModel = 0U;
    }

    if (shaderProgramObject)
    {
        GLsizei shader_count;
        GLuint *p_shaders = NULL;

        glUseProgram(shaderProgramObject);
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint *)malloc(shader_count * sizeof(GLuint));
        memset((void *)p_shaders, 0, shader_count * sizeof(GLuint));

        glGetAttachedShaders(shaderProgramObject, shader_count, &shader_count, p_shaders);

        for (GLsizei i = 0; i < shader_count; i++)
        {
            glDetachShader(shaderProgramObject, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
        glUseProgram(0);
    }

    if (skyboxShaderProgramId)
    {
        GLsizei shader_count;
        GLuint *p_shaders = NULL;

        glUseProgram(skyboxShaderProgramId);
        glGetProgramiv(skyboxShaderProgramId, GL_ATTACHED_SHADERS, &shader_count);

        p_shaders = (GLuint *)malloc(shader_count * sizeof(GLuint));
        memset((void *)p_shaders, 0, shader_count * sizeof(GLuint));

        glGetAttachedShaders(skyboxShaderProgramId, shader_count, &shader_count, p_shaders);

        for (GLsizei i = 0; i < shader_count; i++)
        {
            glDetachShader(skyboxShaderProgramId, p_shaders[i]);
            glDeleteShader(p_shaders[i]);
            p_shaders[i] = 0;
        }

        free(p_shaders);
        p_shaders = NULL;

        glDeleteProgram(skyboxShaderProgramId);
        skyboxShaderProgramId = 0;
        glUseProgram(0);
    }
}

unsigned int loadCubemap(char **faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrComponents, 0);
        if (data)
        {
            fprintf(gpFile, "Loaded texture for %s\n", faces[i]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            fprintf(gpFile, "Cubemap texture failed to load at path: %s", faces[i]);
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


GLuint loadShaders(const char *vertexShaderSourceCode, const char *fragmentShaderSourceCode)
{
    GLuint programId = 0U;

    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

    if (GL_TRUE == compileShader(vertexShaderId, vertexShaderSourceCode))
    {
        GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        if (GL_TRUE == compileShader(fragmentShaderId, fragmentShaderSourceCode))
        {
            programId = glCreateProgram();
            glAttachShader(programId, vertexShaderId);
            glAttachShader(programId, fragmentShaderId);
        }
        else
        {
            glDeleteShader(vertexShaderId);
            glDeleteShader(fragmentShaderId);
        }
    }
    else
    {
        glDeleteShader(vertexShaderId);
    }
    return programId;
}

GLint compileShader(unsigned int shaderId, const char *shaderSourceCode)
{
    GLint status = 0;

    glShaderSource(shaderId, 1, (const GLchar **)&shaderSourceCode, nullptr);
    glCompileShader(shaderId);
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
    if (GL_FALSE == status)
    {
        GLint infoLogLength = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            GLchar *szInfoLog = (GLchar *)malloc(sizeof(GLchar) * (infoLogLength + 1));
            if (nullptr != szInfoLog)
            {
                glGetShaderInfoLog(shaderId, infoLogLength, nullptr, szInfoLog);
                fprintf(gpFile, "Vertex Shader compilation error log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = nullptr;
            }
        }
    }
    return status;
}

GLint linkProgram(GLuint programId)
{
    GLint status   = GL_FALSE;
    GLint nShaders = 0;

    glLinkProgram(programId);

    glUseProgram(programId);
    glGetProgramiv(programId, GL_ATTACHED_SHADERS, &nShaders);
    if (0 < nShaders)
    {
        GLuint *pShaders = new GLuint[nShaders]();
        if (nullptr != pShaders)
        {
            glGetAttachedShaders(programId, nShaders, nullptr, pShaders);
            for (GLint idx = 0U; idx <= nShaders; ++idx)
            {
                glDetachShader(programId, pShaders[idx]);
                glDeleteShader(pShaders[idx]);
                pShaders[idx] = 0U;
            }
            delete[] pShaders;
            pShaders = nullptr;
        }
    }
    glUseProgram(0U);

    glGetProgramiv(programId, GL_LINK_STATUS, &status);
    if (GL_FALSE == status)
    {
        int infoLogLength = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            GLchar *szInfoLog = (GLchar *)malloc(sizeof(GLchar) * (infoLogLength + 1));
            if (nullptr != szInfoLog)
            {
                glGetProgramInfoLog(programId, infoLogLength, nullptr, szInfoLog);
                fprintf(gpFile, "Program linking error log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = nullptr;
            }
        }
    }
    return status;
}
