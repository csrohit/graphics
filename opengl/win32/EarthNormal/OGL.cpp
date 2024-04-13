/**
 * @file    main.cpp
 * @brief   Earth Specular mapping
 * @author  Rohit Nimkar
 * @date    2024-04-10
 * @version 1.1
 */

/*--- Linked libraries ---*/
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")

/*--- System Headers ---*/
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string.h>

/*--- Windows Headers ---*/
#include <Windows.h>

/*--- OpenGL Headers ---*/
#include <GL/glew.h>
#include <GL/gl.h>

/*--- Library headers ---*/
#include "vmath.h"
using namespace vmath;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*--- Program headers ---*/
#include "ogl.h"
#include "load.h"

/* Macro definitions */
#define WIN_WIDTH     800
#define WIN_HEIGHT    600
#define UNUSED_VAL(x) ((void)x)

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMALS,
    AMC_ATTRIBUTE_UVS
};

/*--- FUnction Declaration ---*/
/**
 * @brief Windows Procedure callback function
 *
 * @param hwnd   [in] - Handle to window
 * @param uMsg   [in] - Message identifier
 * @param wParam [in] - word parameter
 * @param lParam [in] - Long parameter
 * @return Success or failure
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
void ToggleFullScreen(void);

/**
 * @brief Resize window
 *
 * @param width  [in] - requested width
 * @param height [in] - Requested height
 */
void resize(int width, int height);

/**
 * @brief Load texture into memory
 *
 * @param filename [in]  - file name
 *
 * @returns texture id
 */
GLuint loadGLTexture(const char* filename);

/**
 * @brief Print OpenGL Driver information
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

/* Global variable declaration */
FILE*           gpFile       = NULL;                      // log file handle
HWND            gHwnd        = NULL;                      // global window handle
BOOL            gbActive     = FALSE;                     // indicate if window is active
DWORD           gdwStyle     = 0;                         // unsigned long 32bit
WINDOWPLACEMENT gwpPrev      = {sizeof(WINDOWPLACEMENT)}; // previous window placement
BOOL            gbFullScreen = FALSE;                     // win32s BOOL not c++ bool
HDC             ghdc         = NULL;                      // global handle for device context
HGLRC           ghrc         = NULL;                      // Global handle for rendering context

/* OpenGL identifiers */
GLuint shaderProgramObject;
GLuint vao         = 0U;
GLuint vboPosition = 0U;
GLuint eboSpheres  = 0U;

/* Matrix uniforms */
GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint cameraPositionUniform;

mat4 projectionMatrix;

/* Texture uniforms  */
GLuint diffuseSamplerUniform;
GLuint normalSamplerUniform;
GLuint specularSamplerUniform;
GLuint cloudSamplerUniform;

GLuint textureDiffuse;
GLuint textureNormal;
GLuint textureSpecular;
GLuint textureClouds;

/* Light uniforms */
GLuint lightAmbientUniform  = 0;
GLuint lightDiffuseUniform  = 0;
GLuint lightSpecularUniform = 0;
GLuint lightPositionUniform = 0;

GLfloat lightAmbient[]  = {0.2f, 0.2f, 0.2f, 1.0f};       // grey ambient light
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

/* Variables */
Model model      = {0};
float lightAngle = 0.0f;
float earthAngle = 0.0f;

bool isAnimationEnabled = true;
/**
 * @brief Entry point function for Win32 Desktop application
 *
 * @param hInstance    [in] - instance of current program
 * @param hPreInstance [in] - always null (except win 3.0)
 * @param lpszCmdLine  [in] - command line parameters
 * @param iCmdShow     [in] - SW_SHOWMINNOACTIVE or SW_SHOWNORMAL
 *
 * @return return value from the application
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    /* Local variables */
    WNDCLASSEX wndclass             = {0};  // window class typw of window , EX dor extended
    HWND       hwnd                 = NULL; // handle though it uint*  its uint only bcoz can't dereference it. opaque ptr
    MSG        msg                  = {0};  // struct
    TCHAR      szAppName[]          = TEXT("Rohit Nimkar: OpenGL Effects");
    int        xDesktopWindowWidth  = 0;     // width of desktop window in pixels
    int        yDesktopWindowHeight = 0;     // height of desktop window in pixels
    int        x                    = 0;     // window leftmost point
    int        y                    = 0;     // window top point
    int        iResult              = 0;     // result of operations
    BOOL       bDone                = FALSE; // flag to indicate message loop compuletion status

    gpFile = fopen("log.txt", "w");
    if (NULL == gpFile)
    {
        MessageBox(NULL, TEXT("Log file can not be opened"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    fprintf(gpFile, "Program for Model Loading started successfully!!!!\n");

    // WNDCLASSEX wndclass initialization predefined classes => button,dialog,scrollbar,status,dynamic
    wndclass.cbSize      = sizeof(WNDCLASSEX);                    // count of bytes in WNDCLASSEX
    wndclass.style       = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;    // redraw after resize (class style)
    wndclass.cbClsExtra  = 0;                                     // count of extra bytes in Class
    wndclass.cbWndExtra  = 0;                                     // count of extra bytes in window
    wndclass.lpfnWndProc = WndProc;                               // windows procedure callback function
    wndclass.hInstance   = hInstance;                             // give current program instance to handl
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // gdi32.dll handle OF Stock brush from os  (getstockobjext => brush,font,paint ret val HGDIIBJ => handle graphic device interface obj )
    wndclass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON)); // to give our icon to our window..make resource from given int which is in rc file
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);                  // load predefined cursor provided by OS
    wndclass.lpszClassName = szAppName;                                    // in custom class classname is mandatory to assign
    wndclass.lpszMenuName  = NULL;                                         // menu name
    wndclass.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON)); // sm = small icon to display icon on taskbar  (new one)

    /* Register class with OS -> os creates unique immutable string for this class of type ATOM */
    (void)RegisterClassEx(&wndclass);

    xDesktopWindowWidth  = GetSystemMetrics(SM_CXSCREEN);
    yDesktopWindowHeight = GetSystemMetrics(SM_CYSCREEN);

    x = (xDesktopWindowWidth - 800) / 2;
    y = (yDesktopWindowHeight - 600) / 2;

    /* create window */
    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,                                                      // extended window style
        szAppName,                                                            // window class name
        TEXT("Earth Normal Map: Rohit Nimkar"),                               // caption
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, // window styles
        x,                                                                    // x co-ordinate of windows top-left corner
        y,                                                                    // y co-ordinate of windows top-left corner
        WIN_WIDTH,                                                            // window width
        WIN_HEIGHT,                                                           // window height
        NULL,                                                                 // handle of parent window
        NULL,                                                                 // handle to menu
        hInstance,                                                            // handle to instance of module associated with window
        NULL);                                                                // Pointer to a value to be passed to the window through the WM_CREATE message.

    // style of window => 6types = WS_OVERLAPPPED | WS_CAPTION | WS_SYSMENU (move/resize..sys bcoz for all windows) | WS_THICKFRAME |WS_NAXIMIZEBOX | WS_MINIMIZEBOX
    gHwnd = hwnd;

    // initialization
    iResult = initialize();
    if (iResult != 0)
    {
        MessageBox(hwnd, TEXT("initialize() failed"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
    }

    /* to show window (handle of window, how to display window => default icmshow */
    ShowWindow(hwnd, iCmdShow);

    SetForegroundWindow(hwnd); // sets window on foreground irrespective of children/sibling window pos
    SetFocus(hwnd);            // keeps window highlighted

    /* game loop */
    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
            else
            {
                UNUSED_VAL(TranslateMessage(&msg));
                UNUSED_VAL(DispatchMessage(&msg));
            }
        }
        else
        {
            if (gbActive == TRUE)
            {
                /* If window is focused then display the window and update state */
                display();
                SwapBuffers(ghdc);
                if (isAnimationEnabled)
                {
                    update();
                }
            }
        }
    }

    /* clean up */
    uninitialize();

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    switch (iMsg)
    {
        case WM_SETFOCUS: /* arrives when window is activated */
        {
            gbActive = TRUE;
            break;
        }
        case WM_KILLFOCUS: /* arrives when other window is activated */
        {
            gbActive = FALSE;
            break;
        }
        case WM_SIZE: /* arrives when window is resized */
        {
            resize(LOWORD(lParam), HIWORD(lParam));
            break;
        }
        case WM_KEYDOWN: /* arrives when non char key is pressed */
        {
            switch (LOWORD(wParam))
            {
                case VK_ESCAPE: /* arrives when escapekey is pressed */
                    /*
                        Deactivate window, remove keyboard focus
                        Flush thread messages queue
                        Destroy Menu and timers

                        If Parent then destroy child windows first
                    */
                    UNUSED_VAL(DestroyWindow(hwnd));
            }
            break;
        }
        case WM_CHAR: /* arrives when char keys are pressed */
        {
            switch (LOWORD(wParam))
            {
                case 'F':
                case 'f':
                    if (gbFullScreen == FALSE)
                    {
                        ToggleFullScreen();
                        gbFullScreen = TRUE;
                        fprintf(gpFile, "F/f key pressed and entered into full screen successfully!!!!\n");
                        break;
                    }
                    else
                    {
                        ToggleFullScreen();
                        gbFullScreen = FALSE;
                        fprintf(gpFile, "F/f key pressed and exited into full screen successfully!!!!\n");
                        break;
                    }
                    break;
                case 'a':
                case 'A':
                {
                    isAnimationEnabled = !isAnimationEnabled;
                    break;
                }
            }
            break;
        }
        case WM_CLOSE:
        {
            // optional if need to run program though window is closed
            UNUSED_VAL(DestroyWindow(hwnd));
            break;
        }
        case WM_DESTROY: // compulsory to handle as unhandled left window alive though prog terminates
        {
            PostQuitMessage(0);
            break;
        }
        default: break;
    }
    return (DefWindowProc(hwnd, iMsg, wParam, lParam)); // default window procedure of os which handles other msges than handles by wndproc
}

void ToggleFullScreen(void)
{
    MONITORINFO mInfo = {sizeof(MONITORINFO)};

    if (gbFullScreen == FALSE)
    {
        gdwStyle = GetWindowLong(gHwnd, GWL_STYLE);

        if (gdwStyle & WS_OVERLAPPEDWINDOW)
        {
            if (GetWindowPlacement(gHwnd, &gwpPrev) && GetMonitorInfo(MonitorFromWindow(gHwnd, MONITORINFOF_PRIMARY), &mInfo))
            {
                UNUSED_VAL(SetWindowLong(gHwnd, GWL_STYLE, (gdwStyle & ~WS_OVERLAPPEDWINDOW)));
                UNUSED_VAL(SetWindowPos(
                    gHwnd, HWND_TOP, mInfo.rcMonitor.left, mInfo.rcMonitor.top, (mInfo.rcMonitor.right - mInfo.rcMonitor.left), (mInfo.rcMonitor.bottom - mInfo.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED));
            }
        }

        UNUSED_VAL(ShowCursor(FALSE));
    }
    else
    {
        UNUSED_VAL(SetWindowPlacement(gHwnd, &gwpPrev));
        UNUSED_VAL(SetWindowLong(gHwnd, GWL_STYLE, (gdwStyle | WS_OVERLAPPEDWINDOW)));
        UNUSED_VAL(SetWindowPos(gHwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED));
        UNUSED_VAL(ShowCursor(TRUE));
    }
}

int initialize(void)
{
    if (0 > loadModel(&model, "sphere.model"))
    {
        fprintf(gpFile, "Failed to load model sphere.model\n");
        return -1;
    }

    fprintf(gpFile, "Model loaded successfully\n");
    PIXELFORMATDESCRIPTOR pfd               = {};
    int                   iPixelFormatIndex = 0;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    /* Step 1: initialization of pixelformat descriptor */
    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // draw on window woth real time
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits   = 8;
    pfd.cBlueBits  = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    /* Step 1: Get Drawing Context from windows */
    ghdc = GetDC(gHwnd);
    if (ghdc == NULL)
    {
        fprintf(gpFile, "GetDC() failed\n");
        return -1;
    }

    /* Step 3: Request matching pixel format from windows */
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    // should return non-zero positive value on success
    if (0 == iPixelFormatIndex)
    {
        fprintf(gpFile, "ChoosePizelFormat() failed\n");
        return -2;
    }

    /* Step 4: set obtained pixel format */
    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFile, "SetPixelFormat() failed\n");
        return -3;
    }

    /* Step 5: Create opengl contexet from device context
               windows opengl bridging api => WGL windows graphics library
               tell WGL  to give me opengl compatible context from this dc
    */

    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL)
    {
        fprintf(gpFile, "wglCreateContext() failed\n");
        return -4;
    }
    //(now ghdc will give up its control and gave it to ghrc)

    /* Step 6: make rendering context current */
    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpFile, "wglMakeCurrent() failed\n");
        return -5;
    }

    if (GLEW_OK != glewInit())
    {
        fprintf(gpFile, "glewInit() failed\n");
        return (-6);
    }

    printGLInfo();

    const GLchar* vertexShaderSourceCode =
        "#version 450 core"
        "\n"
        "in vec3 aPosition;"
        "in vec3 aNormal;"
        "in vec2 aTexCoord;"

        "uniform mat4 uModelMatrix;"
        "uniform mat4 uViewMatrix;"
        "uniform mat4 uProjectionMatrix;"

        "out vec2 oTexCoord;"
        "out vec3 oWorldPosition;"
        "out vec3 oWorldNormal;"
        "out vec3 oLightDirection;"
        "out vec3 oViewDirection;"
        "uniform vec3  uLightPosition;"
        "uniform vec3 uCameraPosition;"

        "void main(void)"
        "{"
        "   oTexCoord       = aTexCoord;"
        "   oWorldPosition  = vec3(uModelMatrix * vec4(aPosition, 1.0f));"
        "   oWorldNormal      = mat3(uModelMatrix) * aNormal;"
        "   oLightDirection = uLightPosition - oWorldPosition;"
        "   oViewDirection   = uCameraPosition - oWorldPosition;"

        "   gl_Position = uProjectionMatrix * uViewMatrix * vec4(oWorldPosition, 1.0f);"
        "}";

    const GLchar* fragmentShaderSourceCode =
        "#version 450 core"
        "\n"
        "in vec2 oTexCoord;"
        "in vec3 oWorldPosition;"
        "in vec3 oWorldNormal;"
        "in vec3 oLightDirection;"
        "in vec3 oViewDirection;"
        "\n"
        "uniform sampler2D uSamplerDiffuse;"
        "uniform sampler2D uSamplerNormal;"
        "uniform sampler2D uSamplerSpecular;"
        "uniform sampler2D uSamplerCloud;"
        "\n"
        "uniform vec3  uMaterialAmbient;"
        "uniform vec3  uMaterialDiffused;"
        "uniform vec3  uMaterialSpecular;"
        "uniform float uMaterialShininess;"
        "\n"
        "uniform vec3  uLightAmbient;"
        "uniform vec3  uLightDiffused;"
        "uniform vec3  uLightSpecular;"
        "\n"
        "out vec4 FragColor;"
        "vec3 getNormalFromMap()"
        "{"
        "   vec3 tangentNormal = texture(uSamplerNormal, oTexCoord).xyz * 2.0f - 1.0f;"
        "   vec3 Q1 = dFdx(oWorldPosition);"
        "   vec3 Q2 = dFdy(oWorldPosition);"
        "   vec2 st1 = dFdx(oTexCoord);"
        "   vec2 st2 = dFdy(oTexCoord);"
        "\n"
        "   vec3 N = normalize(oWorldNormal);"
        "   vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);"
        "   vec3 B = -normalize(cross(N, T));"
        "   mat3 TBN = mat3(T, B, N);"
        "   return (normalize(TBN * tangentNormal));"
        "}"

        "void main(void)"
        "{"
        "    vec3 phongADSLight;"
        "    vec3 normal = getNormalFromMap();"
        "    vec3 color  = texture(uSamplerDiffuse, oTexCoord).rgb;"
        "    vec3 spec   = texture(uSamplerSpecular, oTexCoord).rrr;"
        "    vec3 cloud  = texture(uSamplerCloud, oTexCoord).rgb;"
        "\n"
        "    vec3 nLightVector       = normalize(oLightDirection);"
        "    vec3 nViewVector        = normalize(oViewDirection);"
        "    vec3 nReflectionVector  = reflect(-nLightVector, normal);"
        "\n"
        "    float normalAttenuation    = max(dot(nLightVector, normal), 0.0);"
        "    float reflectedAttenuation = max(dot(nReflectionVector, nViewVector), 0.0);"
        "\n"
        "   vec3 ambientLight  = uLightAmbient * uMaterialAmbient;"
        "   vec3 diffuseLight  = uLightDiffused * color * uMaterialDiffused * normalAttenuation;"
        "   vec3 specularLight = uLightSpecular * spec * pow(reflectedAttenuation, uMaterialShininess);"
        "   vec3 cloudLight    = vec3(0.5f, 0.5f, 0.5f) * cloud * normalAttenuation;"
        "\n"
        "    phongADSLight = ambientLight + diffuseLight + specularLight + cloudLight;"
        "    FragColor     = vec4(phongADSLight, 1.0f);"
        "}";

    shaderProgramObject = loadShaders(vertexShaderSourceCode, fragmentShaderSourceCode);
    if (0U == shaderProgramObject)
    {
        fprintf(gpFile, "Failed to load shaders \n");
        return -1;
    }

    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMALS, "aNormal");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_UVS, "aTexCoord");

    if (GL_TRUE != linkProgram(shaderProgramObject))
    {
        fprintf(gpFile, "Failed to link shaders\n");
        return -1;
    }

    fprintf(gpFile, "----- Shader Program Linked Successfully -----\n");

    // get MVP uniform location
    modelMatrixUniform      = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform       = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");

    lightAmbientUniform  = glGetUniformLocation(shaderProgramObject, "uLightAmbient");
    lightDiffuseUniform  = glGetUniformLocation(shaderProgramObject, "uLightDiffused");
    lightSpecularUniform = glGetUniformLocation(shaderProgramObject, "uLightSpecular");
    lightPositionUniform = glGetUniformLocation(shaderProgramObject, "uLightPosition");

    materialAmbientUniform   = glGetUniformLocation(shaderProgramObject, "uMaterialAmbient");
    materialDiffusedUniform  = glGetUniformLocation(shaderProgramObject, "uMaterialDiffused");
    materialSpecularUniform  = glGetUniformLocation(shaderProgramObject, "uMaterialSpecular");
    materialShininessUniform = glGetUniformLocation(shaderProgramObject, "uMaterialShininess");
    cameraPositionUniform    = glGetUniformLocation(shaderProgramObject, "uCameraPosition");

    normalSamplerUniform   = glGetUniformLocation(shaderProgramObject, "uSamplerNormal");
    diffuseSamplerUniform  = glGetUniformLocation(shaderProgramObject, "uSamplerDiffuse");
    specularSamplerUniform = glGetUniformLocation(shaderProgramObject, "uSamplerSpecular");
    cloudSamplerUniform    = glGetUniformLocation(shaderProgramObject, "uSamplerCloud");

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

    /* Enable Smooth shading */
    glShadeModel(GL_SMOOTH);

    /* Enabling Depth */
    glClearDepth(1.0f);      //[Compulsory] Make all bits in depth buffer as '1'
    glEnable(GL_DEPTH_TEST); //[Compulsory] enable depth test
    glDepthFunc(GL_LEQUAL);  //[Compulsory] Which function to use for testing

    /* Enabling face culling */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    /* Reset Projection Matrix */
    projectionMatrix = mat4::identity();

    /* Load Textures */
    textureDiffuse = loadGLTexture("textures/day.jpg");
    if (0U == textureDiffuse)
    {
        fprintf(gpFile, "Failed to load diffuse texture\n");
        return -1;
    }

    textureNormal = loadGLTexture("textures/normal.png");
    if (0U == textureDiffuse)
    {
        fprintf(gpFile, "Failed to load diffuse texture\n");
        return -1;
    }

    textureSpecular = loadGLTexture("textures/specular.png");
    if (0U == textureDiffuse)
    {
        fprintf(gpFile, "Failed to load diffuse texture\n");
        return -1;
    }

    textureClouds = loadGLTexture("textures/clouds.jpg");
    if (0U == textureDiffuse)
    {
        fprintf(gpFile, "Failed to load diffuse texture\n");
        return -1;
    }

    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void resize(int width, int height)
{
    if (height <= 0)
        height = 1;

    projectionMatrix = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    glViewport(0, 0, width, height);
}

void display(void)
{
    mat4 modelMatrix       = mat4::identity();
    mat4 translationMatrix = mat4::identity();
    mat4 scaleMatrix       = mat4::identity();
    mat4 rotationMatrix    = mat4::identity();
    mat4 viewMatrix        = mat4::identity();
    vec3 cameraPosition    = vec3(0.0f, 0.0f, 3.0f);
    vec3 cameraDirection   = vec3(0.0f, 0.0f, -1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgramObject);
    glBindVertexArray(vao);
    {
        viewMatrix     = lookat(cameraPosition, cameraDirection, vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = rotate(earthAngle, 0.0f, 1.0f, 0.0f);
        modelMatrix    = translate(0.0f, 0.0f, 0.0f);
        modelMatrix    = modelMatrix * rotationMatrix;

        glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);
        glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);

        glUniform3fv(lightPositionUniform, 1, lightPosition);
        glUniform3fv(cameraPositionUniform, 1, cameraPosition);

        glUniform3fv(lightAmbientUniform, 1, lightAmbient);
        glUniform3fv(lightDiffuseUniform, 1, lightDiffused);
        glUniform3fv(lightSpecularUniform, 1, lightSpecular);
        glUniform4fv(lightPositionUniform, 1, lightPosition);

        glUniform3fv(materialAmbientUniform, 1, materialAmbient);
        glUniform3fv(materialDiffusedUniform, 1, materialDiffused);
        glUniform3fv(materialSpecularUniform, 1, materialSpecular);
        glUniform1f(materialShininessUniform, materialShinyness);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureDiffuse);
        glUniform1i(diffuseSamplerUniform, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureNormal);
        glUniform1i(normalSamplerUniform, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureSpecular);
        glUniform1i(specularSamplerUniform, 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureClouds);
        glUniform1i(cloudSamplerUniform, 3);

        glDrawElements(GL_TRIANGLES, model.header.nIndices, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

void update()
{
    if (true == isAnimationEnabled)
    {
        lightAngle += 0.0002;
        if (360.0f < lightAngle)
        {
            lightAngle -= 360.0f;
        }

        lightPosition[0] = 50.0f * cosf(lightAngle);
        lightPosition[1] = 0.0f;
        lightPosition[2] = 50.0f * sinf(lightAngle);

        earthAngle += 0.005;
        if (360.0f < earthAngle)
        {
            earthAngle -= 360.0f;
        }
    }
}

void uninitialize(void)
{
    unloadModel(&model);

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

    if (0U != textureDiffuse)
    {
        glDeleteTextures(1, &textureDiffuse);
        textureDiffuse = 0U;
    }

    if (0U != textureNormal)
    {
        glDeleteTextures(1, &textureNormal);
        textureNormal = 0U;
    }

    if (0U != textureSpecular)
    {
        glDeleteTextures(1, &textureSpecular);
        textureSpecular = 0U;
    }

    if (0U != textureClouds)
    {
        glDeleteTextures(1, &textureClouds);
        textureClouds = 0U;
    }

    if (shaderProgramObject)
    {
        GLsizei nShaders;
        GLuint* pShaders = NULL;

        glUseProgram(shaderProgramObject);
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &nShaders);

        pShaders = (GLuint*)malloc(nShaders * sizeof(GLuint));
        memset((void*)pShaders, 0, nShaders * sizeof(GLuint));

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

    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent((HDC)NULL, (HGLRC)NULL);
    }

    if (NULL != ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = (HGLRC)NULL;
    }

    if (NULL != ghdc)
    {
        ReleaseDC(gHwnd, ghdc);
        ghdc = (HDC)NULL;
    }

    if (NULL != gpFile)
    {
        fprintf(gpFile, "\n----- Program Completed Successfully -----\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

GLuint loadGLTexture(const char* filename)
{
    unsigned char* data      = NULL;
    int            width     = 0;
    int            height    = 0;
    int            nChannels = 0;
    GLenum         format    = GL_RGB;
    GLuint         texture   = 0U;

    stbi_set_flip_vertically_on_load(true);

    data = stbi_load(filename, &width, &height, &nChannels, 0);
    if (data == NULL)
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
        data = NULL;
        fprintf(gpFile, "Texture loaded %s, nChannels %d\n", filename, nChannels);
    }
    return texture;
}

void printGLInfo(void)
{
    GLint nExtensions = 0;
    GLint idx         = 0;

    fprintf(gpFile, "OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
    fprintf(gpFile, "OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    fprintf(gpFile, "OpenGL Version: %s\n", glGetString(GL_VERSION));
    fprintf(gpFile, "OpenGL SL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    /* List supported extensions */
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    for (idx = 0; idx < nExtensions; ++idx)
    {
        fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, idx));
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
        fprintf(gpFile, "Failed to link program: \n");
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