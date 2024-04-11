/**
 * @file    main.cpp
 * @brief   Model loading
 * @author  Rohit Nimkar
 * @date    2024-03-10
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
#include "load.h"

/*--- Macro definitions ---*/
#define gpFILE     stdout
#define WIN_WIDTH  800
#define WIN_HEIGHT 600

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR
};

/*--- Function declarations ---*/
void uninitialize(void);
void toggleFullscreen(void);
int  initialize(void);
void resize(int, int);
void display(void);
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
 * @param texture  [out] - pointer to texture id
 * @param filename [in]  - file name
 *
 * @returns texture id
 */
void loadGLTexture(GLuint* texture, const char* filename);

GLuint loadShader(GLuint shaderType, const char* shaderSource);
GLint  linkProgram(GLuint program);

/* Windowing related variables */
Display*     dpy         = nullptr; // connection to server
Colormap     colormap    = 0UL;
Window       window      = 0UL;     // handle of current window
XVisualInfo* pVisualInfo = nullptr; // Pointer to current visual info
bool         gbAbortFlag = false;   // Global abort flag
GLXContext   glxContext  = nullptr; // handle to OpenGL context
GLboolean    shouldDraw  = false;
FILE*        gpFile      = stdout;

Bool   bFullscreen  = False;
GLuint program      = 0U;
GLuint vao          = 0U;
GLuint vbo_position = 0U;
GLuint eboSphere    = 0U;

vmath::mat4 MVPMatrix        = {};
vmath::mat4 ProjectionMatrix = {};
GLuint      uMVPMatrixId     = 0U;
Model       model            = {0};

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

        update();
        display();

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
        "uniform mat4 uMVPMatrix;"
        "void main(void)"
        "{"
        "    gl_Position =uMVPMatrix * aPosition;"
        "}";

    const GLchar* fragmentShaderSource =
        "#version 460 core"
        "\n"
        "in vec4 oColor;"
        "out vec4 FragColor;"
        "void main(void)"
        "{"
        "    FragColor = vec4(1, 1, 1, 1);"
        "}";

    // clang-format off
    static const GLfloat positions[] = {
        -1.0f, -1.0f, 0.0f, 
         1.0f, -1.0f, 0.0f, 
         0.0f,  1.0f, 0.0f,
    };

    static const GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    // clang-format on

    program = loadShaders(vertexShaderSource, fragmentShaderSource);
    if (0U == program)
    {
        fprintf(gpFile, "Failed to load shaders into memory\n");
        return -1;
    }

    /* Step 7: Bind attaribute location with shader program object */
    glBindAttribLocation(program, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(program, AMC_ATTRIBUTE_COLOR, "aColor");

    // Create and compile our GLSL program from the shaders
    if (0 == linkProgram(program))
    {
        fprintf(gpFILE, "[%s] Failed to link program\n", __func__);
        uninitialize();
        exit(1);
    }
    fprintf(gpFILE, "[%s] Program linked successfully\n", __func__);

    /* VAO */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* Vertex positions */
    glGenBuffers(1, &vbo_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * model.header.nVertices, model.pVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glVertexAttribPointer(
        AMC_ATTRIBUTE_POSITION, // attribute index
        3,                      // nElements per vertex
        GL_FLOAT,               // type
        GL_FALSE,               // OGL should not normalize data, as it is already normalized
        sizeof(Vertex),      // byte offset between generic vertex attributes
        (void*)0                // offset to first vertex
    );

    /* Vertex colors */
    glGenBuffers(1, &eboSphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * model.header.nIndices, model.pIndices, GL_STATIC_DRAW);
    glBindVertexArray(0U);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

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

    ProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    glViewport(0, 0, width, height); // view complete window
}

void display()
{
    static float  angle = 0.0f;
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);

    /* Set the vertextBuffer as active buffer */
    glBindVertexArray(vao);

    vmath::mat4 TranslationMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
    vmath::mat4 ModelViewMatrix   = TranslationMatrix * vmath::rotate(angle, 0.0f, 1.0f, 0.0f);
    MVPMatrix                     = ProjectionMatrix * ModelViewMatrix;
    glUniformMatrix4fv(uMVPMatrixId, 1, GL_FALSE, MVPMatrix[0]);

    /* Draw triangle from data in currently active buffer */
    glDrawElements(GL_TRIANGLES, model.header.nIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0U);

    angle += 0.1f;
    if(angle > 360.0f)
    {
        angle = angle - 360.0f;
    }
}

void update()
{
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

    if (0U != program)
    {
        glUseProgram(program);
        /* Get no. of attached shaders */
        GLint nShaders = 0;
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &nShaders);
        if (0 < nShaders)
        {
            GLuint* pShaders = new GLuint[nShaders]();
            if (nullptr != pShaders)
            {
                glGetAttachedShaders(program, nShaders, nullptr, pShaders);
                for (GLuint idx = 0U; idx <= nShaders; ++idx)
                {
                    glDetachShader(program, pShaders[idx]);
                    glDeleteShader(pShaders[idx]);
                    pShaders[idx] = 0U;
                }
                delete[] pShaders;
                pShaders = nullptr;
            }
        }
        glUseProgram(0U);
        glDeleteProgram(program);
        program = 0U;
    }

    if (0U != vbo_position)
    {
        glDeleteBuffers(1, &vbo_position);
        vbo_position = 0U;
    }

    if (0U != eboSphere)
    {
        glDeleteBuffers(1, &eboSphere);
        eboSphere = 0U;
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
