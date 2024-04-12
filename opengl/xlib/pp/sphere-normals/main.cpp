/**
 * @file    main.cpp
 * @brief   Sphere with Normal visualization
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
GLuint modelMatrixUniform = 0U;
GLuint viewMatrixUniform  = 0U;

GLuint projectionMatrixUniform = 0U;
mat4   projectionMatrix        = {};

/* Functional uniforms */
GLuint keyPressedUniform = 0;
Bool   bLightingEnabled  = False;
Bool   bAnimationEnabled = False;

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
    windowAttributes.colormap          = XCreateColormap(
        dpy,                                  // pointer to display
        RootWindow(dpy, pVisualInfo->screen), // window
        pVisualInfo->visual,                  // pointer to visual
        AllocNone);                           // do not allocate memory, won't need to use this later
    /* Step6: assign this colormap to global colormap */
    colormap = windowAttributes.colormap;

    /* Step7: specify which styles we wish to applied to the window */
    styleMask = CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    /* Step8: create window */
    window = XCreateWindow(
        dpy,                                  // pointer to display
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
                    XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
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
    if (0 > loadModel(&model, "sphere.model"))
    {
        fprintf(gpFile, "Failed to load model sphere.model\n");
        return -1;
    }

    // printModel(&model);
    fprintf(gpFile, "Model loaded successfully\n");

    GLint result = 0; // variable to get value returned by APIS
    glxContext   = glXCreateContext(dpy, pVisualInfo, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, window, glxContext);

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
        "#version 460 core"
        "\n"
        "in vec4 aPosition;"
        "out vec3 oPosition;"
        "\n"
        "uniform int   uKeyPressed;"
        "uniform mat4  uModelMatrix;"
        "uniform mat4  uViewMatrix;"
        "uniform mat4  uProjectionMatrix;"
        "\n"
        "void main(void)"
        "{"
        "    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition;"
        "    oPosition = aPosition.xyz;"
        "}";

    const GLchar* fragmentShaderSource =
        "#version 460 core"
        "\n"
        "in vec3 oPosition;"
        "\n"
        "out vec4 FragColor;"
        "\n"
        "void main(void)"
        "{"
        "        vec3 normal_col = normalize(oPosition) * 0.5 + 0.5;"
        "        FragColor = vec4(normal_col, 1.0);"
        "}";

    shaderProgramObject = loadShaders(vertexShaderSource, fragmentShaderSource);
    if (0U == shaderProgramObject)
    {
        fprintf(gpFile, "Failed to load shaders into memory\n");
        return -1;
    }

    /* Step 7: Bind attaribute location with shader program object */
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMALS, "aNormal");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMALS, "aTexCoord");

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

    const GLfloat uvs[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
    };

    /* Cube */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * model.header.nVertices, model.pVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

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

    /* Enabling Depth */
    glClearDepth(1.0f);      //[Compulsory] Make all bits in depth buffer as '1'
    glEnable(GL_DEPTH_TEST); //[Compulsory] enable depth test
    glDepthFunc(GL_LEQUAL);  //[Compulsory] Which function to use for testing

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glShadeModel(GL_SMOOTH);

    /* warm-up resize */
    XWindowAttributes xattr = {};
    XGetWindowAttributes(dpy, window, &xattr);
    resize(xattr.width, xattr.height);
    return (0);
}

void resize(int32_t width, int32_t height)
{
    if (height <= 0)
        height = 1;

    projectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    glViewport(0, 0, width, height); // view complete window
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window with color whose bit is set, all bits in depth buffer set to 1 (if value is 1 or lower because of LEQUAL)

    vmath::mat4 modelMatrix       = vmath::mat4::identity();
    vmath::mat4 translationMatrix = vmath::mat4::identity();
    vmath::mat4 scaleMatrix       = vmath::mat4::identity();
    vmath::mat4 viewMatrix        = vmath::mat4::identity();

    glUseProgram(shaderProgramObject);
    /* cube */

    glBindVertexArray(vao);
    translationMatrix = vmath::translate(0.0f, 0.0f, -5.0f);

    modelMatrix = translationMatrix * rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);

    glDrawElements(GL_TRIANGLES, model.header.nIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0U);
}

void update()
{
    rotationAngle += 0.5;
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
    GLXContext currentGLXContext = NULL;
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

    if (0U != vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0U;
    }

    currentGLXContext = glXGetCurrentContext();
    if ((NULL != currentGLXContext) && (currentGLXContext == glxContext))
    {
        glXMakeCurrent(dpy, 0, NULL);
    }

    if (NULL != glxContext)
    {
        glXDestroyContext(dpy, glxContext);
        glxContext = NULL;
    }

    if (NULL != pVisualInfo)
    {
        free(pVisualInfo);
        pVisualInfo = NULL;
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

    if (NULL != dpy)
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

void loadGLTexture(GLuint* texture, const char* filename)
{
    unsigned char* data      = NULL;
    int            width     = 0;
    int            height    = 0;
    int            nChannels = 0;
    GLenum         format    = GL_RGB;

    stbi_set_flip_vertically_on_load(true);

    data = stbi_load(filename, &width, &height, &nChannels, 0);
    if (data == NULL)
    {
        fprintf(gpFile, "Error : failed to load texture %s.\n", filename);
        return;
    }

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
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    // set up texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // push the data to texture memory
    glTexImage2D(GL_TEXTURE_2D, 0, format, (GLint)width, (GLint)height, 0, format, GL_UNSIGNED_BYTE, (const void*)data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    data = NULL;
    fprintf(gpFile, "Texture loaded %s\n", filename);
}