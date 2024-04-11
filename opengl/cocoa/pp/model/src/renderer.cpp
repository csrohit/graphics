#include "renderer.h"

#include <OpenGL/gl3.h>
#include <cstdlib>
#include <cstring>

GLint  compileShader(unsigned int shaderId, const char *shaderSourceCode);
GLint  linkProgram(GLuint programId);
GLuint loadShaders(const char *vertexShaderSourceCode, const char *fragmentShaderSourceCode);

const GLchar *vertexShaderSource = "#version 410 core"
                                   "\n"
                                   "in vec3 aPosition;"
                                   "in vec3 aColor;"
                                   "out vec3 oColor;"
                                   "void main(void)"
                                   "{"
                                   "   gl_Position = vec4(aPosition, 1.0f);"
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

GLfloat positions[] = {
    0.0f,  1.0f,  0.0f, // top
    -0.5f, -0.5f, 0.0f, // bottom-left
    0.5f,  -0.5f, 0.0f  // bottom-right
};

const GLfloat colors[] = {
    1.0f, 0.0f, 0.0f, // red
    0.0f, 1.0f, 0.0f, /// green
    0.0f, 0.0f, 1.0f  // blue
};
GLuint vao         = 0U;
GLuint vboPosition = 0U;
GLuint vboColor    = 0U;

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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);
    glBindVertexArray(0);

    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glClearDepth(1.0f);

    return (0);
}

void onKeyDown(int key)
{
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramObject);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void update()
{
}

void resize()
{
}

void uninitialize()
{
    if (0U != vboPosition)
    {
        glDeleteBuffers(1, &vboPosition);
        vboPosition = 0U;
    }

    if (0U != vboColor)
    {
        glDeleteBuffers(1, &vboColor);
        vboColor = 0U;
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