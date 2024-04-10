#include "renderer.h"

#include <OpenGL/gl3.h>
#include <cstdlib>
#include <cstring>
#include "vmath.h"
using namespace vmath;

#define INCREMENT_ANGLE 0.1f;


GLint        compileShader(unsigned int shaderId, const char *shaderSourceCode);
GLint        linkProgram(GLuint programId);
unsigned int loadShaders(const char *vertexShaderSourceCode, const char *fragmentShaderSourceCode);


const GLchar *vertexShaderSource = "#version 410 core"
                                   "\n"
                                   "in  vec3 aPosition;"
                                   "in  vec3 aColor;"
                                   "out vec3 oColor;"
                                   "\n"
                                   "uniform mat4 uModelMatrix;"
                                   "uniform mat4 uViewMatrix;"
                                   "uniform mat4 uProjectionMatrix;"
                                   "void main(void)"
                                   "{"
                                   "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0f);"
                                   "   oColor = aColor;"
                                   "}";

const GLchar *fragmentShaderSource = "#version 410 core"
                                     "\n"
                                     "in vec3 oColor;"
                                     "out vec4 FragColor;"
                                     "void main(void)"
                                     "{"
                                     "   FragColor = vec4(oColor, 1.0f);"
                                     "}";

GLuint shaderProgramObject = 0U;
float  positions[]         = {-1.0f, -1.0f, 1.0f,  //        7--------6
                              1.0f,  -1.0f, 1.0f,  //       /|       /|
                              1.0f,  -1.0f, -1.0f, //      4--------5 |
                              -1.0f, -1.0f, -1.0f, //      | |      | |
                              -1.0f, 1.0f,  1.0f,  //      | 3------|-2
                              1.0f,  1.0f,  1.0f,  //      |/       |/
                              1.0f,  1.0f,  -1.0f, //      0--------1
                              -1.0f, 1.0f,  -1.0f};

unsigned int indices[] = {
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

GLuint vao         = 0U;
GLuint vboPosition = 0U;
GLuint eboCube     = 0U;

/* Uniforms */
GLuint projectionMatrixUniform = 0U;
GLuint modelMatrixUniform      = 0U;
GLuint viewMatrixUniform       = 0U;

/* Uniform data binding */
mat4 projectionMatrix = {0};

/* Functional variables */
GLfloat rotationAngle = 0.0f;
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

    glBindAttribLocation(shaderProgramObject, 0, "aPosition");
    glBindAttribLocation(shaderProgramObject, 1, "aColor");

    if (GL_FALSE == linkProgram(shaderProgramObject))
    {
        fprintf(gpFile, "Failed to link shaders\n");
        glDeleteProgram(shaderProgramObject);
        return -1;
    }

    fprintf(gpFile, "Shader program linked successfully\n");

    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
    modelMatrixUniform      = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform       = glGetUniformLocation(shaderProgramObject, "uViewMatrix");

    fprintf(gpFile, "MVP Matrix %u\n", projectionMatrixUniform);
    fprintf(gpFile, "MVP Matrix %u\n", modelMatrixUniform);
    fprintf(gpFile, "MVP Matrix %u\n", viewMatrixUniform);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    glVertexAttrib3f(1, 1.0f, 1.0f, 1.0f);
    glGenBuffers(1, &eboCube);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboCube);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0U);

    projectionMatrix = mat4::identity();

    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glClearDepth(1.0f);

    return (0);
}

void onKeyDown(int key)
{
}

void display()
{
    mat4 modelMatrix       = mat4::identity();
    mat4 viewMatrix        = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 scaleMatrix       = mat4::identity();
    mat4 rotationMatrix    = mat4::identity();

    /* Transformation Matrix */
    rotationMatrix = rotate(rotationAngle, 1.0f, 0.0f, 0.0f);
    rotationMatrix = rotationMatrix * rotate(rotationAngle, 0.0f, 1.0f, 0.0f);
    rotationMatrix = rotationMatrix * rotate(rotationAngle, 0.0f, 0.0f, 1.0f);
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
    viewMatrix  = lookat(vec3(0.0f, 0.0f, 6.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

void update()
{
    rotationAngle = rotationAngle + INCREMENT_ANGLE;
    if(360.0f < rotationAngle)
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
    if (0U != vboPosition)
    {
        glDeleteBuffers(1, &vboPosition);
        vboPosition = 0U;
    }

    if (0U != eboCube)
    {
        glDeleteBuffers(1, &eboCube);
        eboCube = 0U;
    }

    if (0U != vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0U;
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
}


unsigned int loadShaders(const char *vertexShaderSourceCode, const char *fragmentShaderSourceCode)
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
