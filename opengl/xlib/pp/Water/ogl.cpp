/**
 * @file   ogl.cpp
 * @brief  Per Vertex Shading
 * @author Rohit Nimkar
 * @date   19/04/2024
 */

/*--- System Headers ---*/
#include <memory.h> //memset()
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*--- XWindows Headers ---*/
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/*--- OpenGL headers ---*/
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

/*--- Library headers ---*/
#include "vmath.h"
using namespace vmath;

#include "Sphere.h"

/*--- Macro Definitions ---*/
#define WIN_WIDTH  1600U
#define WIN_HEIGHT 1200U
#define FBO_WIDTH  512
#define FBO_HEIGHT 512
#define N_LIGHTS   3

/*--- Type declarations ---*/
enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD
};

typedef struct Light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 position;
} Light;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int *);

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
 * @brief Display contexts on screen
 */
void display(void);

/**
 * @brief Update contents on screen
 */
void update(void);

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
 * @brief Print Driver Information
 */
void printGLInfo(void);

/**
 * @brief Compile shader
 *
 * @param shaderId     [in] - shader identifier
 * @param shaderSource [in] - shader source code
 *
 * @returns 0 on success else negative value
 */
GLint compileShader(unsigned int shaderId, const char *shaderSource);

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
GLuint loadShaders(const char *vertexSource, const char *fragmentSource);

Bool CreateFBO(GLint textureWidth, GLint textureHeight);


/*--- Global variable declarations ---*/
FILE        *gpFile        = NULL;
Display     *dpy           = NULL;
XVisualInfo *pVisualInfo   = NULL;
Colormap     colormap      = 0UL;
Window       window        = 0UL;
bool         bFullscreen   = false;
Bool         bActiveWindow = False;

/*--- OpenGL Context variables ---*/
GLXContext                     glxContext                 = NULL;
GLXFBConfig                    glxFBConfig                = {};
glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;


/*--- OpenGL Program variables ---*/
GLuint sphereShaderObject = 0U;

/*--- Shader data ---*/
GLuint vaoSphere          = 0U;
GLuint vboSpherePositions = 0U;
GLuint vboSphereNormals   = 0U;
GLuint vboSphereTexCoords = 0U;
GLuint eboSphere          = 0U;

GLfloat        spherePositions[1146];
GLfloat        sphereNormals[1146];
GLfloat        sphereTexCoords[764];
unsigned short sphereIndices[2280];

int gnSphereVertices = 0U;
int gnSphereIndices  = 0U;

/*--- Matrix Uniforms ---*/
GLuint modelMatrixUniformSphere      = 0U;
GLuint viewMatrixUniformSphere       = 0U;
GLuint projectionMatrixUniformSphere = 0U;

mat4 sphereProjectionMatrix = {0};

/*--- Lights ---*/
GLuint lightAmbientUniform[N_LIGHTS]  = {0};
GLuint lightDiffuseUniform[N_LIGHTS]  = {0};
GLuint lightSpecularUniform[N_LIGHTS] = {0};
GLuint lightPositionUniform[N_LIGHTS] = {0};

Light lights[N_LIGHTS] = {};

/*--- Materials ---*/
GLuint materialAmbientUniform   = 0U;
GLuint materialDiffuseUniform   = 0U;
GLuint materialSpecularUniform  = 0U;
GLuint materialShininessUniform = 0U;

vec3  materialAmbient   = vec3(0.0f, 0.0f, 0.0f);
vec3  materialDiffuse   = vec3(1.0f, 1.0f, 1.0f);
vec3  materialSpecular  = vec3(1.0f, 1.0f, 1.0f);
float materialShininess = 50.0f;

/*--- Functional ---*/
GLuint keyPressedUniform = 0U;
Bool   keyPressed        = False;

GLfloat rotationAngle     = 0.0f;
Bool    bAnimationEnabled = False;
GLint   winWidth          = 0;
GLint   winHeight         = 0;

/* FBO */
GLuint fbo = 0U;
GLuint rbo = 0U;

GLuint fboSamplerUniform = 0U;
GLuint textureFbo        = 0U;
Bool   bFboResult        = False;

int main(void)
{
    int                  defaultScreen    = 0;
    XSetWindowAttributes windowAttributes = {0};
    int                  styleMask        = 0;
    Atom                 wmDelete         = 0;
    XEvent               event            = {0};
    KeySym               keySym           = 0UL;
    int                  screenWidth      = 0;
    int                  screenHeight     = 0;
    char                 keys[26]         = {0};
    Bool                 bDone            = False;
    XRectangle           rect             = {0}; // window dimentions rectangle

    /* PP Variables */
    GLXFBConfig *glxFBConfigs           = NULL;
    GLXFBConfig  bestGLXFBConfig        = {};
    XVisualInfo *tempXVisualInfo        = NULL;
    int          nFBConfigs             = 0;
    int          bestFrameBufferConfig  = -1;
    int          bestNumberOfSamples    = -1;
    int          worstFrameBufferConfig = -1;
    int          worstNumberOfSamples   = 999;
    int          sampleBuffers          = 0;
    int          nSamples               = 0;
    int          idx                    = 0U;

    // clang-format off
    int frameBufferAttributes[] = {
        GLX_X_RENDERABLE,  True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE,      8,
        GLX_GREEN_SIZE,    8, 
        GLX_BLUE_SIZE,     8,
        GLX_ALPHA_SIZE,    8,
        GLX_DEPTH_SIZE,    24,
        GLX_STENCIL_SIZE,  8,
        GLX_DOUBLEBUFFER,  True,
        None
    };
    // clang-format on

    gpFile = fopen("log.txt", "w");
    if (NULL == gpFile)
    {
        printf("Log file cannot be created\n");
        exit(0);
    }
    fprintf(gpFile, "Program started successfully\n");

    /* establish connection with x-server */
    dpy = XOpenDisplay(NULL);
    if (NULL == dpy)
    {
        printf("%s: XOpenDisplay failed\n", __func__);
        uninitialize();
        exit(1);
    }

    /* get default screen from display */
    defaultScreen = XDefaultScreen(dpy);

    /* Get available FB configs */
    glxFBConfigs = glXChooseFBConfig(dpy, defaultScreen, frameBufferAttributes, &nFBConfigs);
    if (NULL == glxFBConfigs)
    {
        fprintf(gpFile, "Matching glxFBConfigs could not be found\n");
        uninitialize();
        exit(1);
    }

    /* Find best matching FBConfig from above array */
    for (idx = 0; idx < nFBConfigs; ++idx)
    {
        tempXVisualInfo = glXGetVisualFromFBConfig(dpy, glxFBConfigs[idx]);
        if (NULL != tempXVisualInfo)
        {
            glXGetFBConfigAttrib(dpy, glxFBConfigs[idx], GLX_SAMPLE_BUFFERS, &sampleBuffers);
            glXGetFBConfigAttrib(dpy, glxFBConfigs[idx], GLX_SAMPLES, &nSamples);

            if (bestFrameBufferConfig < 0 || sampleBuffers && nSamples > bestNumberOfSamples)
            {
                bestFrameBufferConfig = idx;
                bestNumberOfSamples   = nSamples;
            }

            if (worstFrameBufferConfig < 0 || !sampleBuffers || nSamples < worstNumberOfSamples)
            {
                worstFrameBufferConfig = idx;
                worstNumberOfSamples   = nSamples;
            }
            XFree(tempXVisualInfo);
            tempXVisualInfo = NULL;
        }
    }

    /* Accordingly get best GLXFBConfig */
    bestGLXFBConfig = glxFBConfigs[bestFrameBufferConfig];

    /* assign to global FB config */
    glxFBConfig = bestGLXFBConfig;

    /* Free memory given to array */
    XFree(glxFBConfigs);

    /* Get best visual from FB config */
    pVisualInfo = glXGetVisualFromFBConfig(dpy, glxFBConfig);
    if (NULL == pVisualInfo)
    {
        printf("glXGetVisualFromFBConfig failed\n");
        uninitialize();
        exit(1);
    }

    /* Step5: Set window properties */
    memset((void *)&windowAttributes, 0, sizeof(XSetWindowAttributes));
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
    }

    /* Step9: State which events we want to receive */
    XSelectInput(dpy, window, ExposureMask | VisibilityChangeMask | StructureNotifyMask | KeyPressMask | ButtonPressMask | PointerMotionMask | FocusChangeMask);

    wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW",
                           True); // cristopher tronche, kenton lee, hpux

    /* Step10: add/set as protocol for window manager */
    XSetWMProtocols(dpy, window, &wmDelete, 1);

    /* Step11: set window title/caption/name */
    XStoreName(dpy, window, "RTR5: Rohit Nimkar");

    /* Step12: Show window */
    XMapWindow(dpy, window);

    screenWidth  = XWidthOfScreen(XScreenOfDisplay(dpy, pVisualInfo->screen));
    screenHeight = XHeightOfScreen(ScreenOfDisplay(dpy, pVisualInfo->screen));
    XMoveWindow(dpy, window, (screenWidth - WIN_WIDTH) / 2, (screenHeight - WIN_HEIGHT) / 2);

    /*--- OpenGL initialization ---*/
    int res = initialize();
    if (0 != res)
    {
        fprintf(gpFile, "Initialize failed\n");
        uninitialize();
        exit(res);
    }
    /* Event Loop */
    while (!bDone)
    {
        while (XPending(dpy))
        {
            (void)XNextEvent(dpy, &event);
            switch (event.type)
            {
                case KeyPress:
                    keySym = XkbKeycodeToKeysym(dpy,                // pointer to display
                                                event.xkey.keycode, // actual keycode
                                                0,                  // key sym group
                                                0                   // is shift pressed
                    );
                    switch (keySym)
                    {
                        case XK_Escape:
                        {
                            bDone = True;
                            break;
                        }
                        default:
                            break;
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
                        case 'l':
                        case 'L':
                        {
                            keyPressed = !keyPressed;
                            break;
                        }
                        case 'a':
                        case 'A':
                        {
                            bAnimationEnabled = !bAnimationEnabled;
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case 33:
                {
                    bDone = True;
                    break;
                }
                case FocusIn:
                {
                    bActiveWindow = True;
                    break;
                }
                case FocusOut:
                {
                    bActiveWindow = False;
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
                case ButtonPress:
                {
                    break;
                }
                default:
                    break;
            }
        }

        /*--- rendering ---*/
        if (!bActiveWindow)
            continue;

        display();
        glXSwapBuffers(dpy, window);

        if (bAnimationEnabled)
        {
            update();
        }
    }

    uninitialize();
    return (0);
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
    (void)memset((void *)&event, 0, sizeof(XEvent));
    event.type                 = ClientMessage;
    event.xclient.window       = window;
    event.xclient.message_type = wmStateNormal;
    event.xclient.format       = 32;
    event.xclient.data.l[0]    = 2;
    event.xclient.data.l[1]    = wmStateFullscreen;

    /* Step4: send event*/
    XSendEvent(dpy,                                  // pointer to display
               RootWindow(dpy, pVisualInfo->screen), // root window of application
               False,                                // should this event be passed to child windows
               SubstructureNotifyMask,               // which subevent to swnd along with
                                                     // ClientMessage
               &event                                // pointer to event
    );

    /* Step4: */
    bFullscreen = !bFullscreen;
}

GLuint initializeSphere()
{
    GLuint        shaderProgramObject      = 0U;
    const GLchar *sphereVertexShaderSource = "#version 460 core"
                                             "\n"
                                             "#define N_LIGHTS 3"
                                             "\n"
                                             "in  vec4 aPosition;"
                                             "in  vec3 aNormal;"
                                             "\n"
                                             "out vec3 wNormal;"
                                             "out vec3 wCameraDirection;"
                                             "out vec3 wLightDirection[N_LIGHTS];"
                                             "\n"
                                             "uniform mat4 uModelMatrix;"
                                             "uniform mat4 uViewMatrix;"
                                             "uniform mat4 uProjectionMatrix;"
                                             "\n"
                                             "uniform vec4 uLightPosition[N_LIGHTS];"
                                             "\n"
                                             "uniform int uKeyPressed;"
                                             "\n"
                                             "void main(void)"
                                             "{"
                                             "    vec4 ePosition = uViewMatrix * uModelMatrix * aPosition;"
                                             "    if(uKeyPressed == 1)"
                                             "    {"
                                             "        mat3 normalMatrix = mat3(transpose(inverse(uViewMatrix * uModelMatrix)));"
                                             "\n"
                                             "        wNormal              = normalMatrix * aNormal;"
                                             "        wCameraDirection     = -ePosition.xyz;"
                                             "        for (int i = 0; i < N_LIGHTS; i++)"
                                             "        {"
                                             "            wLightDirection[i] = vec3(uLightPosition[i] - ePosition);"
                                             "        }"
                                             "    }"
                                             "    else"
                                             "    {"
                                             "        wLightDirection[0]  = vec3(0.0f, 0.0f, 0.0f);"
                                             "        wLightDirection[1]  = vec3(0.0f, 0.0f, 0.0f);"
                                             "        wLightDirection[2]  = vec3(0.0f, 0.0f, 0.0f);"
                                             "        wNormal          = vec3(0.0f, 0.0f, 0.0f);"
                                             "        wCameraDirection = vec3(0.0f, 0.0f, 0.0f);"
                                             "    }"
                                             "    gl_Position = uProjectionMatrix * ePosition;"
                                             "}";

    const GLchar *sphereFragmentShaderSource = "#version 460 core"
                                               "\n"
                                               "#define N_LIGHTS 3"
                                               "\n"
                                               "in  vec3 wNormal;"
                                               "in  vec3 wCameraDirection;"
                                               "in  vec3 wLightDirection[N_LIGHTS];"
                                               "out vec4 FragColor;"
                                               "\n"
                                               "uniform vec3 uLightAmbient[N_LIGHTS];"
                                               "uniform vec3 uLightDiffuse[N_LIGHTS];"
                                               "uniform vec3 uLightSpecular[N_LIGHTS];"
                                               "\n"
                                               "uniform vec3 uMaterialAmbient;"
                                               "uniform vec3 uMaterialDiffuse;"
                                               "uniform vec3 uMaterialSpecular;"
                                               "uniform float uMaterialShininess;"
                                               "\n"
                                               "uniform int uKeyPressed;"
                                               "void main(void)"
                                               "{"
                                               "    vec3 oColor = vec3(1.0, 1.0, 1.0);"
                                               "    if(uKeyPressed == 1)"
                                               "    {"
                                               "        oColor = vec3(0.0, 0.0, 0.0);"
                                               "        vec3 nwNormal              = normalize(wNormal);"
                                               "        vec3 nwCameraDirection     = normalize(wCameraDirection);"
                                               "        for (int i = 0; i < N_LIGHTS; i++)"
                                               "        {"
                                               "            vec3 nwLightDirection      = normalize(wLightDirection[i]);"
                                               "            vec3 nwReflectionDirection = reflect(-nwLightDirection, nwNormal);"
                                               "\n"
                                               "            vec3 ambient  = uLightAmbient[i] * uMaterialAmbient;"
                                               "            vec3 diffuse  = uLightDiffuse[i] * uMaterialDiffuse * max(dot(nwLightDirection, nwNormal), 0.0f);"
                                               "            vec3 specular = uLightSpecular[i] * uMaterialSpecular * pow(max(dot(nwReflectionDirection, nwCameraDirection), 0.0f), uMaterialShininess);"
                                               "            oColor += ambient + diffuse + specular;"
                                               "        }"
                                               "    }"
                                               "    FragColor = vec4(oColor, 1.0f);"
                                               "}";

    shaderProgramObject = loadShaders(sphereVertexShaderSource, sphereFragmentShaderSource);
    if (0U == shaderProgramObject)
    {
        fprintf(gpFile, "Failed to load shaders for sphere\n");
        return 0;
    }


    /* Step 7: Bind attaribute location with shader program object */
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "aNormal");

    if (GL_TRUE != linkProgram(shaderProgramObject))
    {
        fprintf(gpFile, "Failed to link shaders for sphere\n");
        return 0;
    }

    fprintf(gpFile, "----- Shader Program Linked Successfully for sphere -----\n");

    /* Setup up uniforms */
    modelMatrixUniformSphere      = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniformSphere       = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniformSphere = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");

    lightAmbientUniform[0]  = glGetUniformLocation(shaderProgramObject, "uLightAmbient[0]");
    lightDiffuseUniform[0]  = glGetUniformLocation(shaderProgramObject, "uLightDiffuse[0]");
    lightSpecularUniform[0] = glGetUniformLocation(shaderProgramObject, "uLightSpecular[0]");
    lightPositionUniform[0] = glGetUniformLocation(shaderProgramObject, "uLightPosition[0]");

    lightAmbientUniform[1]  = glGetUniformLocation(shaderProgramObject, "uLightAmbient[1]");
    lightDiffuseUniform[1]  = glGetUniformLocation(shaderProgramObject, "uLightDiffuse[1]");
    lightSpecularUniform[1] = glGetUniformLocation(shaderProgramObject, "uLightSpecular[1]");
    lightPositionUniform[1] = glGetUniformLocation(shaderProgramObject, "uLightPosition[1]");

    lightAmbientUniform[2]  = glGetUniformLocation(shaderProgramObject, "uLightAmbient[2]");
    lightDiffuseUniform[2]  = glGetUniformLocation(shaderProgramObject, "uLightDiffuse[2]");
    lightSpecularUniform[2] = glGetUniformLocation(shaderProgramObject, "uLightSpecular[2]");
    lightPositionUniform[2] = glGetUniformLocation(shaderProgramObject, "uLightPosition[2]");

    materialAmbientUniform   = glGetUniformLocation(shaderProgramObject, "uMaterialAmbient");
    materialDiffuseUniform   = glGetUniformLocation(shaderProgramObject, "uMaterialDiffuse");
    materialSpecularUniform  = glGetUniformLocation(shaderProgramObject, "uMaterialSpecular");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "uMaterialShininess");

    keyPressedUniform = glGetUniformLocation(shaderProgramObject, "uKeyPressed");

    /* Load sphere data */
    getSphereVertexData(spherePositions, sphereNormals, sphereTexCoords, sphereIndices);
    gnSphereVertices = getNumberOfSphereVertices();
    gnSphereIndices  = getNumberOfSphereElements();

    /* Sphere */
    glGenVertexArrays(1, &vaoSphere);
    glBindVertexArray(vaoSphere);

    glGenBuffers(1, &vboSpherePositions);
    glBindBuffer(GL_ARRAY_BUFFER, vboSpherePositions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(spherePositions), spherePositions, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vboSphereNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphereNormals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereNormals), sphereNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &eboSphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereIndices), sphereIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    lights[0].ambient = vmath::vec3(0.0f, 0.0f, 0.0f);
    lights[1].ambient = vmath::vec3(0.0f, 0.0f, 0.0f);
    lights[2].ambient = vmath::vec3(0.0f, 0.0f, 0.0f);

    lights[0].diffuse = vmath::vec3(1.0f, 0.0f, 0.0f);
    lights[1].diffuse = vmath::vec3(0.0f, 1.0f, 0.0f);
    lights[2].diffuse = vmath::vec3(0.0f, 0.0f, 1.0f);

    lights[0].specular = vmath::vec3(1.0f, 0.0f, 0.0f);
    lights[1].specular = vmath::vec3(0.0f, 1.0f, 0.0f);
    lights[2].specular = vmath::vec3(0.0f, 0.0f, 1.0f);

    sphereProjectionMatrix = mat4::identity();
    return shaderProgramObject;
}

int initializeWaterSurface(void)
{
    const GLchar *waterSurfaceVertexShaderSource   = "#version 460 core"
                                                     "\n"
                                                     "void main()"
                                                     "{"
                                                     "in vec4 aPosition;"
                                                     "in vec2 aTexCoord;"
                                                     "}";

    const GLchar *waterSurfaceFragmentShaderSource = "#version 460 core"
                                                     "\n"
                                                     "in vec2 vTexCoord;"
                                                     "uniform sampler2D samplerReflection;"
                                                     "void main()"
                                                     "{"
                                                     "    vec4 colorReflection = texture(samplerReflection, vTexCoord);"
                                                     "    FragColor = colorReflection;"
                                                     "}";
}

int initialize(void)
{
    // clang-format off
    int attribsNew[] = 
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 5,
        GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB, None
    };

    int attribsOld[] = 
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        None
    };
    // clang-format on

    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
    if (NULL == glXCreateContextAttribsARB)
    {
        printf("failed to get function\n");
        uninitialize();
        return -1;
    }

    glxContext = glXCreateContextAttribsARB(dpy, glxFBConfig, 0, True, attribsNew);
    if (NULL == glxContext)
    {
        fprintf(gpFile, "Core profile based glxContext cannot be obtained, falling "
                        "back to old context\n");

        /* Getting old context */
        glxContext = glXCreateContextAttribsARB(dpy, glxFBConfig, 0, True, attribsOld);
        if (NULL == glxContext)
        {
            fprintf(gpFile, "Old glxContext cannot be found\n");
            return -1;
        }
        else
        {
            fprintf(gpFile, "Old glxContext found\n");
        }
    }
    else
    {
        fprintf(gpFile, "Core profile based glxContext obtained successfully\n");
    }

    if (0 == glXIsDirect(dpy, glxContext))
    {
        fprintf(gpFile, "Not supporting direct rendering or hardware rendering\n");
    }
    else
    {
        fprintf(gpFile, "Hardware rendering is supported\n");
    }

    /* make this context as current context */
    if (False == glXMakeCurrent(dpy, window, glxContext))
    {
        fprintf(gpFile, "glXMakeCurrent failed\n");
        return (-1);
    }

    if (GLEW_OK != glewInit())
    {
        fprintf(gpFile, "glewInit() failed\n");
        return (-6);
    }

    /* FBO related code */
    // if (CreateFBO(FBO_WIDTH, FBO_HEIGHT) == True)
    // {
    //     bFboResult = True;
    //     fprintf(gpFile, "Failed to create FBO\n");
    // }

    sphereShaderObject = initializeSphere();
    if (0U == sphereShaderObject)
    {
        fprintf(gpFile, "Failed to initialize for sphere\n");
        return -6;
    }

    /* Enable Depth testing */
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    /* Enable face culling */
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /* Global Warm up resize */
    XWindowAttributes xattr = {};
    XGetWindowAttributes(dpy, window, &xattr);
    resize(xattr.width, xattr.height);

    return (0);
}

void resize(int width, int height)
{
    if (height == 0)
        height = 1;

    winWidth  = width;
    winHeight = height;

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    sphereProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void displaySphere(void)
{
    mat4 modelMatrix       = mat4::identity();
    mat4 viewMatrix        = mat4::identity();
    mat4 rotationMatrix    = mat4::identity();
    mat4 translationMatrix = mat4::identity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(sphereShaderObject);

    glUniformMatrix4fv(projectionMatrixUniformSphere, 1, GL_FALSE, sphereProjectionMatrix);

    viewMatrix = lookat(vec3(0.0f, 0.0f, 12.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(viewMatrixUniformSphere, 1, GL_FALSE, viewMatrix);

    if (keyPressed == True)
    {
        glUniform1i(keyPressedUniform, 1);

        glUniform3fv(lightAmbientUniform[0], 1, lights[0].ambient);
        glUniform3fv(lightDiffuseUniform[0], 1, lights[0].diffuse);
        glUniform3fv(lightSpecularUniform[0], 1, lights[0].specular);
        glUniform4fv(lightPositionUniform[0], 1, lights[0].position);

        glUniform3fv(lightAmbientUniform[1], 1, lights[1].ambient);
        glUniform3fv(lightDiffuseUniform[1], 1, lights[1].diffuse);
        glUniform3fv(lightSpecularUniform[1], 1, lights[1].specular);
        glUniform4fv(lightPositionUniform[1], 1, lights[1].position);

        glUniform3fv(lightAmbientUniform[2], 1, lights[2].ambient);
        glUniform3fv(lightDiffuseUniform[2], 1, lights[2].diffuse);
        glUniform3fv(lightSpecularUniform[2], 1, lights[2].specular);
        glUniform4fv(lightPositionUniform[2], 1, lights[2].position);

        glUniform3fv(materialAmbientUniform, 1, materialAmbient);
        glUniform3fv(materialDiffuseUniform, 1, materialDiffuse);
        glUniform3fv(materialSpecularUniform, 1, materialSpecular);
        glUniform1f(materialShininessUniform, materialShininess);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    glBindVertexArray(vaoSphere);

    translationMatrix = translate(0.0f, 1.0f, 0.0f);
    modelMatrix       = translationMatrix;
    glUniformMatrix4fv(modelMatrixUniformSphere, 1, GL_FALSE, modelMatrix);
    glDrawElements(GL_TRIANGLES, gnSphereIndices, GL_UNSIGNED_SHORT, NULL);

    /* Render reflection */

    viewMatrix = lookat(vec3(0.0f, 0.0f, 12.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
    glUniformMatrix4fv(viewMatrixUniformSphere, 1, GL_FALSE, viewMatrix);
    glDrawElements(GL_TRIANGLES, gnSphereIndices, GL_UNSIGNED_SHORT, NULL);


    glBindVertexArray(0);

    glUseProgram(0);
}

void display()
{
    displaySphere();
}

void update()
{
    rotationAngle = rotationAngle + 0.1;
    if (360.0f < rotationAngle)
    {
        rotationAngle = rotationAngle - 360.0f;
    }

    float radius = 100.0f;

    float c = radius * cosf(rotationAngle);
    float s = radius * sinf(rotationAngle);

    lights[0].position[0] = c;
    lights[0].position[1] = 0.0f;
    lights[0].position[2] = s;

    lights[1].position[0] = 0.0f;
    lights[1].position[1] = c;
    lights[1].position[2] = s;

    lights[2].position[0] = c;
    lights[2].position[1] = s;
    lights[2].position[2] = 0.0f;
}

void uninitializeShader(GLuint shaderProgramObject)
{
    GLsizei nShaders;
    GLuint *pShaders = NULL;

    glUseProgram(shaderProgramObject);
    glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &nShaders);

    pShaders = (GLuint *)malloc(nShaders * sizeof(GLuint));
    memset((void *)pShaders, 0, nShaders * sizeof(GLuint));

    glGetAttachedShaders(shaderProgramObject, nShaders, &nShaders, pShaders);

    for (GLsizei i = 0; i < nShaders; i++)
    {
        glDetachShader(shaderProgramObject, pShaders[i]);
        glDeleteShader(pShaders[i]);
        pShaders[i] = 0;
    }

    free(pShaders);
    pShaders = NULL;

    glDeleteProgram(shaderProgramObject);
    shaderProgramObject = 0;
    glUseProgram(0);
}

void uninitialize(void)
{
    GLXContext currentGLXContext = NULL;
    if (0U != textureFbo)
    {
        fprintf(gpFile, "%s: deleting texture FBO %d", __func__, textureFbo);
        glDeleteTextures(1, &textureFbo);
        textureFbo = 0U;
    }

    if (0U != fbo)
    {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0U;
    }

    if (0U != rbo)
    {
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0U;
    }

    if (0U != vaoSphere)
    {
        glDeleteVertexArrays(1, &vaoSphere);
        vaoSphere = 0;
    }

    if (0U != vboSpherePositions)
    {
        glDeleteBuffers(1, &vboSpherePositions);
        vboSpherePositions = 0;
    }

    if (0U != vboSphereNormals)
    {
        glDeleteBuffers(1, &vboSphereNormals);
        vboSphereNormals = 0;
    }

    if (0U != vboSphereTexCoords)
    {
        glDeleteBuffers(1, &vboSphereTexCoords);
        vboSphereTexCoords = 0;
    }

    if (0U != sphereShaderObject)
    {
        uninitializeShader(sphereShaderObject);
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

    if (NULL != gpFile)
    {
        fprintf(gpFile, "Program terminated successfully\n");
        fclose(gpFile);
    }
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
            fprintf(gpFile, "Failed to compile fragment shader: \n");
            glDeleteShader(vertexShaderId);
            glDeleteShader(fragmentShaderId);
        }
    }
    else
    {
        fprintf(gpFile, "Failed to compile vertex shader: \n");
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
        fprintf(gpFile, "Failed to link program: \n");
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

Bool CreateFBO(GLint textureWidth, GLint textureHeight)
{
    GLint maxRenderBufferSize = 0;

    /* Step 1: Check capacity of Render Buffer */
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderBufferSize);

    fprintf(gpFile, "Max Render buffer size %d\n", maxRenderBufferSize);
    if (maxRenderBufferSize < textureWidth || maxRenderBufferSize < textureHeight)
    {
        fprintf(gpFile, "Texture size overflow\n");
        return False;
    }

    /* Step 2: Create custom frame buffer*/
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    /* Step 3: Create texture in which we are going to render sphere */
    glGenTextures(1, &textureFbo);
    glBindTexture(GL_TEXTURE_2D, textureFbo);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

    /* Attach above texture tp framebuffer at efault color attachment 0 */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureFbo, 0);

    /* CReate render buffer to hold depth */
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    /* Set storage of above render buffer of texture size for depth */
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, textureWidth, textureHeight);

    /* Attach above render buffer for depth to fbo */
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    /* Check frame buffer status, wether successfull or not */
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(gpFile, "Framebuffer creation status is not complete\n");
        return False;
    }

    /* Unbind with the framebuffer */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return True;
}
