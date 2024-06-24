/**
 * @file    main.cpp
 * @brief   Earth Specular mapping
 * @author  Rohit Nimkar
 * @date    2024-04-10
 * @version 1.1
 */

/*--- System Headers ---*/
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string.h>

/*--- XWindows Headers ---*/
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

/*--- OpenGL headers ---*/
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <vector>

/*--- Program headers ---*/
#include "vmath.h"
using namespace vmath;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "load.h"

/*--- Macro definitions ---*/
#define gpFILE     stdout
#define WIN_WIDTH  800
#define WIN_HEIGHT 600

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_UVS,
    AMC_ATTRIBUTE_NORMALS
};

/*--- Function declarations ---*/
/**
 * @brief initilize OpenGL context
 *
 * @return
 */
int initialize(void);

/**
 * @brief UnInitialize OpenGL context
 */
void uninitialize(void);

/**
 * @brief toggle  full screen
 */
void toggleFullscreen(void);

/**
 * @brief Resize window
 *
 * @param width  [in] - requested width
 * @param height [in] - Requested height
 */
void resize(int width, int height);

/**
 * @brief Display contexts on screen
 */
void display(void);

/**
 * @brief Update contents on screen
 */
void update(void);

/**
 * @brief Compile shader
 *
 * @param shaderId     [in] - shader identifier
 * @param shaderSource [in] - shader source code
 *
 * @returns 0 on success else negative value
 */
GLint compileShader(unsigned int shaderId, const char* shaderSource);

/**
 * @brief Link program
 *
 * @param programId [in] - program identifier
 *
 * @returns 0 on success else negative value
 */
GLint linkProgram(GLuint programId);

/**
 * @brief Load shaders into memory
 *
 * @param vertexSource   [in] - Vertex Shader source code
 * @param fragmentSource [in] - Fragment shader source code
 *
 * @returns program id
 */
GLuint loadShaders(const char* vertexSource, const char* fragmentSource);

/**
 * @brief Load texture into memory
 *
 * @param filename [in]  - file name
 *
 * @returns texture id
 */
GLuint loadGLTexture(const char* filename);

void GenerateSphere(float radius, float sectorCount, float stackCount);
/* Windowing related variables */
Display*     dpy         = nullptr; // connection to server
Colormap     colormap    = 0UL;
Window       window      = 0UL;     // handle of current window
XVisualInfo* pVisualInfo = nullptr; // Pointer to current visual info
bool         gbAbortFlag = false;   // Global abort flag
GLXContext   glxContext  = nullptr; // handle to OpenGL context
GLboolean    shouldDraw  = false;
FILE*        gpFile      = stdout;
Bool         bFullscreen = False;

/* OpenGL identifiers */
GLuint shaderProgramObject = 0U;

GLuint vao         = 0U;
GLuint vboPosition = 0U;
GLuint eboSpheres  = 0U;

/* Matrix uniforms */
GLuint modelMatrixUniform      = 0U;
GLuint viewMatrixUniform       = 0U;
GLuint projectionMatrixUniform = 0U;
GLuint viewPosUniform          = 0U;

mat4 projectionMatrix = {};

/* Light uniforms */
GLuint lightAmbientUniform  = 0;
GLuint lightDiffuseUniform  = 0;
GLuint lightSpecularUniform = 0;
GLuint lightPositionUniform = 0;

GLfloat lightAmbient[]  = {0.1f, 0.1f, 0.1f, 1.0f};       // grey ambient light
GLfloat lightDiffused[] = {1.0f, 1.0f, 1.0f, 1.0f};       // white diffused light
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};       // specular color
GLfloat lightPosition[] = {100.0f, 100.0f, 100.0f, 1.0f}; // position of light

/* Material uniforms */
GLuint materialAmbientUniform   = 0;
GLuint materialSpecularUniform  = 0;
GLuint materialDiffusedUniform  = 0;
GLuint materialShininessUniform = 0;

GLfloat materialAmbient[]  = {0.2f, 0.2f, 0.2f, 0.2f}; // ambient reflectance
GLfloat materialDiffused[] = {1.0f, 1.0f, 1.0f, 1.0f}; // diffuse reflectance
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f}; // specular reflectance
GLfloat materialShinyness  = 128.0f;                   // concentration of specular component

GLuint diffuseTextureUniform;
GLuint specularTextureUniform;
GLuint normalTextureUniform;
GLuint textureDiffuse;
GLuint textureSpecular;
GLuint textureNormal;

/* Functional uniforms */
GLuint keyPressedUniform   = 0;
Bool   bLightingEnabled    = False;
Bool   bAnimationEnabled   = False;
GLuint vao_sphere          = 0U;
GLuint vbo_sphere_position = 0U;
GLuint vbo_sphere_normal   = 0U;
GLuint vbo_sphere_texcoord = 0U;
GLuint vbo_sphere_indices  = 0U;

std::vector<float> vertices;
std::vector<float> normals;
std::vector<float> texcoords;
std::vector<int>   indices;

/* Variables */
Model model         = {0};
float rotationAngle = 0.0f;

int main()
{
    int                  defaultScreen    = 0;
    XSetWindowAttributes windowAttributes = {0};
    int                  styleMask        = 0;
    Atom                 wmDelete         = 0;
    XEvent               event            = {0};
    int                  screenWidth      = 0;
    int                  screenHeight     = 0;
    char                 keys[26]         = {0};
    XRectangle           rect             = {0}; // window dimentions rectangle

    // clang-format off
    GLint glxAttriutes[] = {
        GLX_DOUBLEBUFFER, True,
        GLX_RGBA,
        GLX_RED_SIZE,     8,
        GLX_GREEN_SIZE,   8, 
        GLX_BLUE_SIZE,    8,
        GLX_ALPHA_SIZE,   8,
        GLX_DEPTH_SIZE,   8,
        GLX_STENCIL_SIZE, 8,
        None
    };
    // clang-format on

    /*Step 1:  establish connection with x-server */
    dpy = XOpenDisplay(nullptr);
    if (dpy == nullptr)
    {
        printf("%s: XOpenDisplay failed\n", __func__);
        uninitialize();
        exit(1);
    }

    /*Step2:  get default screen from display */
    defaultScreen = XDefaultScreen(dpy);

    /* Step4: get visual */
    pVisualInfo = glXChooseVisual(dpy, defaultScreen, glxAttriutes);
    if (pVisualInfo == nullptr)
    {
        printf("glXChooseVisual failed\n");
        uninitialize();
        exit(1);
    }

    /* Step5: Set window properties */
    memset((void*)&windowAttributes, 0, sizeof(XSetWindowAttributes));
    windowAttributes.border_pixel      = 0UL;
    windowAttributes.background_pixel  = XBlackPixel(dpy, pVisualInfo->screen);
    windowAttributes.background_pixmap = 0UL;
    windowAttributes.colormap          = XCreateColormap(dpy,                                  // pointer to display
                                                         RootWindow(dpy, pVisualInfo->screen), // window
                                                         pVisualInfo->visual,                  // pointer to visual
                                                         AllocNone);                           // do not allocate memory, won't need to use this later
    /* Step6: assign this colormap to global colormap */
    colormap = windowAttributes.colormap;

    /* Step7: specify which styles we wish to applied to the window */
    styleMask = CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    /* Step8: create window */
    window = XCreateWindow(dpy,                                  // pointer to display
                           RootWindow(dpy, pVisualInfo->screen), // parent window [Desktop]
                           0,                                    // x
                           0,                                    // y
                           WIN_WIDTH,                            // window width
                           WIN_HEIGHT,                           // window dimentions
                           0,                                    // border width
                           pVisualInfo->depth,                   // depth
                           InputOutput,                          // class of window
                           pVisualInfo->visual,                  // visual
                           styleMask,                            // which styles to be applied
                           &windowAttributes                     // window attributes
    );

    if (0UL == window)
    {
        printf("XCreateWindow failed\n");
        uninitialize();
        exit(1);
    } // pointer to attributes structure

    /* Step9: State which events we want to receive */
    XSelectInput(dpy, window, ExposureMask | VisibilityChangeMask | StructureNotifyMask | KeyPressMask | ButtonPressMask | PointerMotionMask | FocusChangeMask);

    wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", True); // cristopher tronche, kenton lee, hpux

    /* Step10: add/set as protocol for window manager */
    XSetWMProtocols(dpy, window, &wmDelete, 1);

    /* Step11: set window title/caption/name */
    XStoreName(dpy, window, "Rohit Nimkar: xWindows");

    /* register for window close event */
    wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, window, &wmDelete, 1);

    /* Step12: Show window */
    XMapWindow(dpy, window);

    screenWidth  = XWidthOfScreen(XScreenOfDisplay(dpy, pVisualInfo->screen));
    screenHeight = XHeightOfScreen(ScreenOfDisplay(dpy, pVisualInfo->screen));
    XMoveWindow(dpy, window, (screenWidth - WIN_WIDTH) / 2, (screenHeight - WIN_HEIGHT) / 2);

    /*--- OpenGL initialization ---*/
    int res = initialize();
    if (res < 0)
    {
        printf("initialize failed\n");
        uninitialize();
        exit(res);
    }

    shouldDraw = false;
    while (!gbAbortFlag)
    {
        XEvent event;
        if (XPending(dpy))
        {
            XNextEvent(dpy, &event);
            switch (event.type)
            {
                case Expose:
                {
                    if (!shouldDraw)
                        shouldDraw = true;
                    break;
                }
                case ClientMessage:
                {
                    if (event.xclient.data.l[0] == wmDelete)
                    {
                        gbAbortFlag = true;
                    }

                    break;
                }
                case KeyPress:
                {
                    KeySym sym = XkbKeycodeToKeysym(dpy, event.xkey.keycode, 0, 0);

                    switch (sym)
                    {
                        case XK_Escape:
                        {
                            gbAbortFlag = true;
                            break;
                        }
                    }
                    XLookupString(&event.xkey, keys, sizeof(keys), nullptr, nullptr);
                    switch (keys[0])
                    {
                        case 'f':
                        case 'F':
                        {
                            toggleFullscreen();
                            break;
                        }
                        case 'L':
                        case 'l':
                        {
                            if (bLightingEnabled)
                            {
                                bLightingEnabled = False;
                            }
                            else
                            {
                                bLightingEnabled = True;
                            }
                            break;
                        }
                        break;
                        case 'A':
                        case 'a':
                        {
                            if (bAnimationEnabled)
                            {
                                bAnimationEnabled = False;
                            }
                            else
                            {
                                bAnimationEnabled = True;
                            }
                            break;
                        }

                        default: break;
                    }
                    break;
                }
                case ConfigureNotify:
                {
                    if (rect.width != event.xconfigure.width || rect.height != event.xconfigure.height)
                    {
                        resize(event.xconfigure.width, event.xconfigure.height);
                        rect.width  = event.xconfigure.width;
                        rect.height = event.xconfigure.height;
                    }
                    break;
                }
            }
        }

        if (!shouldDraw)
            continue;

        display();

        if (True == bAnimationEnabled)
        {
            update();
        }

        glXSwapBuffers(dpy, window);
    }

    uninitialize();
    return (0);
}

int initialize()
{
    if (0 > loadModel(&model, "res/plane.model"))
    {
        fprintf(gpFile, "Failed to load model plane.model\n");
        return -1;
    }

    fprintf(gpFile, "Model loaded successfully\n");
    fprintf(gpFile, "Model loaded successfully\n");

    GLint result = 0; // variable to get value returned by APIS
    glxContext   = glXCreateContext(dpy, pVisualInfo, nullptr, GL_TRUE);
    fprintf(gpFile, "12121212Model lfasfasasfoaded successfully\n");
    glXMakeCurrent(dpy, window, glxContext);

    fprintf(gpFile, "Model lfasfasasfoaded successfully\n");
    /* initialize glew */
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        fprintf(gpFILE, "%s: failed to initialize glew\n", __func__);
        uninitialize();
        return -1;
    }

    fprintf(gpFILE, "%-20s:%s\n", "GPU Vendor", glGetString(GL_VENDOR));
    fprintf(gpFILE, "%-20s:%s\n", "Version String", glGetString(GL_VERSION));
    fprintf(gpFILE, "%-20s:%s\n", "Graphics Renderer", glGetString(GL_RENDERER));
    fprintf(gpFILE, "%-20s:%s\n", "GL Shading Language", glGetString(GL_SHADING_LANGUAGE_VERSION));

    /* Program related variables */
    const GLchar* vertexShaderSource =
        "#version 450 core"
        "\n"
        "in vec3 aPosition;"
        "uniform mat4 uModelMatrix;"
        "uniform mat4 uViewMatrix;"
        "uniform mat4 uProjectionMatrix;"
        "void main(void)"
        "{"
        "   gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0f);"
        "}";

    const GLchar* fragmentShaderSource =
        "#version 450 core"
        "\n"
        "out vec4 FragColor;"
        "void main(void)"
        "{"
        "   FragColor = vec4(1.0f, 1.0f, 1.0f , 1.0f);"
        "}";

    shaderProgramObject = loadShaders(vertexShaderSource, fragmentShaderSource);
    if (0U == shaderProgramObject)
    {
        fprintf(gpFile, "Failed to load shaders into memory\n");
        return -1;
    }

    /* Step 7: Bind attaribute location with shader program object */
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");

    // Create and compile our GLSL program from the shaders
    if (0 == linkProgram(shaderProgramObject))
    {
        fprintf(gpFILE, "[%s] Failed to link program\n", __func__);
        uninitialize();
        exit(1);
    }
    fprintf(gpFILE, "[%s] Program linked successfully\n", __func__);
    modelMatrixUniform      = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform       = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");

    /* Cube */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * model.header.nVertices, model.pVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glVertexAttribPointer(AMC_ATTRIBUTE_NORMALS, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMALS);

    glVertexAttribPointer(AMC_ATTRIBUTE_UVS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texel));
    glEnableVertexAttribArray(AMC_ATTRIBUTE_UVS);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    glGenBuffers(1, &eboSpheres);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSpheres);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * model.header.nIndices, model.pIndices, GL_STATIC_DRAW);

    /*
        The order of unbinding is important --
        If Element buffer is unbound before vertex array buffer then we are indicating OpenGl
        that we do not intend to use element buffer
     */
    glBindVertexArray(0U);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // setup vao and vbo
    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);

    glGenBuffers(1, &vbo_sphere_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_position);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_sphere_normal);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_normal);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMALS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMALS);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_sphere_texcoord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere_texcoord);
    glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(AMC_ATTRIBUTE_UVS, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_UVS);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_sphere_indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_sphere_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    /* Enabling Depth */
    glClearDepth(1.0f);      //[Compulsory] Make all bits in depth buffer as '1'
    glEnable(GL_DEPTH_TEST); //[Compulsory] enable depth test
    glDepthFunc(GL_LEQUAL);  //[Compulsory] Which function to use for testing

    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glShadeModel(GL_SMOOTH);

    /* warm-up resize */
    XWindowAttributes xattr = {};
    XGetWindowAttributes(dpy, window, &xattr);
    resize(xattr.width, xattr.height);
    return (0);
}

void resize(int width, int height)
{
    if (height <= 0)
        height = 1;

    projectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    glViewport(0, 0, width, height);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vmath::mat4 modelMatrix       = vmath::mat4::identity();
    vmath::mat4 translationMatrix = vmath::mat4::identity();
    vmath::mat4 scaleMatrix       = vmath::mat4::identity();
    vmath::mat4 viewMatrix        = vmath::mat4::identity();
    vec3        cameraPosition    = vec3(15.0f, 5.0f, 0.0f);
    vec3        cameaDirection    = vec3(0.0f, 0.0f, 0.0f);

    glUseProgram(shaderProgramObject);
    glBindVertexArray(vao);
    {
        viewMatrix = lookat(cameraPosition, cameaDirection, vec3(0.0f, 1.0f, 0.0f));

        modelMatrix = translate(0.0f, 0.0f, 0.0f) * rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);

        // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glDrawElements(GL_TRIANGLES, model.header.nIndices, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
    glBindVertexArray(0U);
}

void update()
{
    rotationAngle += 1;
    if (360.0f < rotationAngle)
    {
        rotationAngle -= 360.0f;
    }
}

void toggleFullscreen(void)
{
    XEvent event;
    Atom   wmStateNormal     = 0UL;
    Atom   wmStateFullscreen = 0UL;

    /* Step1: Create normal state Atom*/
    wmStateNormal = XInternAtom(dpy, "_NET_WM_STATE", False);

    /* Step2: Create fullscreen state Atom*/
    wmStateFullscreen = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);

    /* Step3: fill event structure */
    (void)memset((void*)&event, 0, sizeof(XEvent));
    event.type                 = ClientMessage;
    event.xclient.window       = window;
    event.xclient.message_type = wmStateNormal;
    event.xclient.format       = 32;
    event.xclient.data.l[0]    = 2;
    event.xclient.data.l[1]    = wmStateFullscreen;

    /* Step4: send event*/
    XSendEvent(
        dpy,                                  // pointer to display
        RootWindow(dpy, pVisualInfo->screen), // root window of application
        False,                                // should this event be passed to child windows
        SubstructureNotifyMask,               // which subevent to swnd along with ClientMessage
        &event                                // pointer to event
    );

    /* Step4: */
    bFullscreen = !bFullscreen;
}

void uninitialize()
{
    unloadModel(&model);

    GLXContext currentGLXContext = nullptr;
    if (0U != shaderProgramObject)
    {
        glUseProgram(shaderProgramObject);
        /* Get no. of attached shaders */
        GLint nShaders = 0;
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &nShaders);
        if (0 < nShaders)
        {
            GLuint* pShaders = new GLuint[nShaders]();
            if (nullptr != pShaders)
            {
                glGetAttachedShaders(shaderProgramObject, nShaders, nullptr, pShaders);
                for (GLuint idx = 0U; idx <= nShaders; ++idx)
                {
                    glDetachShader(shaderProgramObject, pShaders[idx]);
                    glDeleteShader(pShaders[idx]);
                    pShaders[idx] = 0U;
                }
                delete[] pShaders;
                pShaders = nullptr;
            }
        }
        glUseProgram(0U);
        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0U;
    }

    /* cube */
    if (0U != vboPosition)
    {
        glDeleteBuffers(1, &vboPosition);
        vboPosition = 0U;
    }

    if (0U != eboSpheres)
    {
        glDeleteBuffers(1, &eboSpheres);
        eboSpheres = 0U;
    }

    if (0U != textureDiffuse)
    {
        glDeleteTextures(1, &textureDiffuse);
        textureDiffuse = 0U;
    }

    if (0U != textureSpecular)
    {
        glDeleteTextures(1, &textureSpecular);
        textureSpecular = 0U;
    }

    if (0U != textureNormal)
    {
        glDeleteTextures(1, &textureNormal);
        textureNormal = 0U;
    }

    if (0U != vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0U;
    }

    currentGLXContext = glXGetCurrentContext();
    if ((nullptr != currentGLXContext) && (currentGLXContext == glxContext))
    {
        glXMakeCurrent(dpy, 0, nullptr);
    }

    if (nullptr != glxContext)
    {
        glXDestroyContext(dpy, glxContext);
        glxContext = nullptr;
    }

    if (nullptr != pVisualInfo)
    {
        free(pVisualInfo);
        pVisualInfo = nullptr;
    }

    if (0UL < colormap)
    {
        XFreeColormap(dpy, colormap);
        colormap = 0UL;
    }

    if (0UL != window)
    {
        XUnmapWindow(dpy, window);
        XDestroyWindow(dpy, window);
        window = 0UL;
    }

    if (nullptr != dpy)
    {
        XCloseDisplay(dpy);
    }
}

GLuint loadShaders(const char* vertexShaderSourceCode, const char* fragmentShaderSourceCode)
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
            fprintf(gpFile, "Fragment shader compilation failed\n");
            glDeleteShader(vertexShaderId);
            glDeleteShader(fragmentShaderId);
        }
    }
    else
    {
        fprintf(gpFile, "Vertex shader compilation failed\n");
        glDeleteShader(vertexShaderId);
    }
    return programId;
}

GLint compileShader(unsigned int shaderId, const char* shaderSourceCode)
{
    GLint status = 0;

    glShaderSource(shaderId, 1, (const GLchar**)&shaderSourceCode, nullptr);
    glCompileShader(shaderId);
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
    if (GL_FALSE == status)
    {
        GLint infoLogLength = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            GLchar* szInfoLog = (GLchar*)malloc(sizeof(GLchar) * (infoLogLength + 1));
            if (nullptr != szInfoLog)
            {
                glGetShaderInfoLog(shaderId, infoLogLength, nullptr, szInfoLog);
                fprintf(gpFile, "Shader compilation error log: %s\n", szInfoLog);
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
    glGetProgramiv(programId, GL_LINK_STATUS, &status);
    if (GL_FALSE == status)
    {
        int infoLogLength = 0;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            GLchar* szInfoLog = (GLchar*)malloc(sizeof(GLchar) * (infoLogLength + 1));
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

GLuint loadGLTexture(const char* filename)
{
    unsigned char* data      = nullptr;
    int            width     = 0;
    int            height    = 0;
    int            nChannels = 0;
    GLenum         format    = GL_RGB;
    GLuint         texture   = 0U;

    stbi_set_flip_vertically_on_load(true);

    data = stbi_load(filename, &width, &height, &nChannels, 0);
    if (data == nullptr)
    {
        fprintf(gpFile, "Error : failed to load texture %s.\n", filename);
    }
    else
    {
        if (nChannels == 1)
        {
            format = GL_RED;
        }
        else if (nChannels == 3)
        {
            format = GL_RGB;
        }
        else if (nChannels == 4)
        {
            format = GL_RGBA;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // set up texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        // push the data to texture memory
        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLint)width, (GLint)height, 0, format, GL_UNSIGNED_BYTE, (const void*)data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
        data = nullptr;
        fprintf(gpFile, "Texture loaded %s, nChannels %d\n", filename, nChannels);
    }
    return texture;
}

void GenerateSphere(float radius, float sectorCount, float stackCount)
{
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;
    int   t1, t2;

    float sectorStep = 2.0f * M_PI / sectorCount;
    float stackStep  = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; i++)
    {
        stackAngle = M_PI / 2.0f - i * stackStep;
        xy         = radius * cosf(stackAngle);
        z          = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; j++)
        {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // normals
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);

            // texcoords
            s = (float)j / sectorCount;
            t = (float)i / stackCount;

            texcoords.push_back(s);
            texcoords.push_back(t);
        }
    }

    for (int i = 0; i < stackCount; i++)
    {
        t1 = i * (sectorCount + 1);
        t2 = t1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; j++, t1++, t2++)
        {
            if (i != 0)
            {
                indices.push_back(t1);
                indices.push_back(t2);
                indices.push_back(t1 + 1);
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(t1 + 1);
                indices.push_back(t2);
                indices.push_back(t2 + 1);
            }
        }
    }
}
