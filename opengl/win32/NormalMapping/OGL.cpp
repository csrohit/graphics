/**
 * @file    ogl.cpp
 * @brief   Color pyramid
 * @author  Rohit Nimkar
 * @date    28/01/2024
 * @version 1.1
 */

/* Windows Header files */
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "vmath.h"

/* OpenGL header files */
#include <gl/glew.h> // this must be before gl/GL.h
#include <gl/GL.h>
#include "ogl.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <cstdint>

/* Macro definitions */
#define WIN_WIDTH     800
#define WIN_HEIGHT    600
#define UNUSED_VAL(x) ((void)x)

using namespace vmath;

/* Link with OpenGl libraries */
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD,
    AMC_ATTRIBUTE_TANGENT
};

/**
 * @brief Windows Procedure callback function
 *
 * @param hwnd   [in] - Handle to window
 * @param uMsg   [in] - Message identifier
 * @param wParam [in] - word parameter
 * @param lParam [in] - Long parameter
 * @return Success or failure
 */
LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
void         resize(int width, int height);
unsigned int loadTexture(char const* path);

void printGLInfo(void);

/* Global variable declaration */
FILE*           gpFILE       = NULL;
FILE*           gpFile       = NULL;
HWND            gHwnd        = NULL; // global window handle
BOOL            gbActive     = FALSE;
DWORD           gdwStyle     = 0;                         // unsigned long 32bit
WINDOWPLACEMENT gwpPrev      = {sizeof(WINDOWPLACEMENT)}; // previous window placement
BOOL            gbFullScreen = FALSE;                     // win32s BOOL not c++ bool
HDC             ghdc         = NULL;                      // global handle for device context
HGLRC           ghrc         = NULL;                      // Global handle for rendering context

const GLchar* vertexShaderSourceCode =
    "#version 460 core"
    "\n"
    "in vec3 aPosition;"
    "in vec3 aNormal;"
    "in vec2 aTexCoord;"
    "in vec3 aTangent;"
    "\n"
    "out vec2 oTexCoord;"
    "out vec3 oLightDirection;"
    "out vec3 oViewDirection;"
    "\n"
    "uniform mat4 uModelMatrix;"
    "uniform mat4 uViewMatrix;"
    "uniform mat4 uProjectionMatrix;"
    "uniform vec3 uLightPosition;"
    "uniform vec3 uViewPosition;"
    "\n"
    "void main(void)"
    "{"
    "\n"
    "   mat3 normalMatrix = transpose(inverse(mat3(uModelMatrix)));"
    "   vec3 T = normalize(normalMatrix * aTangent);"
    "   vec3 N = normalize(normalMatrix * aNormal);"
    "   T = normalize(T - dot(T, N) * N);"
    "   vec3 B = cross(N, T);"
    "   mat3 TBN = transpose(mat3(T, B, N));"
    "\n"
    "   vec3 oTangentLightPos = TBN * uLightPosition;"
    "   vec3 oTangentViewPos  = TBN * uViewPosition;"
    "   vec3 tangentFragPos   = vec3(uModelMatrix * vec4(aPosition, 1.0f));"
    "   oLightDirection = oTangentLightPos - tangentFragPos;"
    "   oViewDirection  = oTangentViewPos - tangentFragPos;"
    "\n"
    "   gl_Position = uProjectionMatrix * uViewMatrix * vec4(tangentFragPos, 1.0f);"
    "   oTexCoord = aTexCoord;"
    "}";

const GLchar* fragmentShaderSourceCode =
    "#version 460 core"
    "\n"
    "in vec2 oTexCoord;"
    "in vec3 oLightDirection;"
    "in vec3 oViewDirection;"
    "\n"
    "out vec4 FragColor;"
    "\n"
    "uniform sampler2D uDiffuseSampler;"
    "uniform sampler2D uSpecularSampler;"
    "uniform sampler2D uNormalSampler;"
    "\n"
    "uniform vec3 uLightAmbient;"
    "uniform vec3 uLightDiffused;"
    "uniform vec3 uLightSpecular;"
    "\n"
    "void main(void)"
    "{"
    "   vec3 normal = normalize(texture(uNormalSampler, oTexCoord).rgb * 2.0f - 1.0f);"
    "   vec3 color  = texture(uDiffuseSampler, oTexCoord).rgb;"
    "   vec3 spec1  = texture(uSpecularSampler, oTexCoord).rgb;"
    "\n"
    "   vec3 lightDirection = normalize(oLightDirection);"
    "   vec3 view_direction = normalize(oViewDirection);"
    "   vec3 reflect_direction = reflect(-lightDirection, normal);"
    "   vec3 halfway_direction = normalize(lightDirection + view_direction);"
    "\n"
    "   vec3 ambient  = uLightAmbient * color;"
    "   vec3 diffuse  = uLightDiffused * color * max(dot(lightDirection, normal), 0.0f);"
    "   vec3 specular = uLightSpecular * spec1 * pow(max(dot(normal, halfway_direction), 0.0f), 50);"

    "   vec3 finalLight = ambient + diffuse + specular;"
    "   FragColor = vec4(finalLight, 1.0f);"
    "}";

GLuint vertexShaderObject   = 0U;
GLuint fragmentShaderObject = 0U;
GLuint shaderProgramObject  = 0U;

const GLfloat vertices[] = {
    -1.0f, 1.0f,  0.0f, // top-left
    -1.0f, -1.0f, 0.0f, // bottom-left
    1.0f,  -1.0f, 0.0f, // bottom-right
    1.0f,  1.0f,  0.0f, // top-right
};

const GLfloat normals[] = {
    0.0f, 0.0f, 1.0f, // top-left
    0.0f, 0.0f, 1.0f, // bottom-left
    0.0f, 0.0f, 1.0f, // bottom-right
    0.0f, 0.0f, 1.0f, // top-right
};

const GLfloat texCoords[] = {
    0.0f, 1.0f, // top-left
    0.0f, 0.0f, // bottom-left
    1.0f, 0.0f, // bottom-right
    1.0f, 1.0f  // top-right
};

const GLfloat tangents[] = {
    2.0f, 0.0f, 0.0f, // top-left
    2.0f, 0.0f, 0.0f, // bottom-left
    2.0f, 0.0f, 0.0f, // bottom-right
    2.0f, 0.0f, 0.0f, // top-right
};

const GLuint indices[] = {
    0, 1, 2, // bottom-left
    2, 3, 0  // top-right
};

GLuint vao          = 0U;
GLuint vboPosition  = 0U;
GLuint eboSquare    = 0U;
GLuint vboNormal    = 0U;
GLuint vboTangent   = 0U;
GLuint vboTexCoords = 0U;

/* Matrix uniforms */
GLuint modelMatrixUniform      = 0U;
GLuint viewMatrixUniform       = 0U;
GLuint projectionMatrixUniform = 0U;
GLuint viewPosUniform          = 0U;

/* Light uniforms */
GLuint lightAmbientUniform  = 0;
GLuint lightDiffuseUniform  = 0;
GLuint lightSpecularUniform = 0;
GLuint lightPositionUniform = 0;

/* Texture uniforms */
GLuint normalSamplerUniform   = 0U;
GLuint diffuseSamplerUniform  = 0U;
GLuint specularSamplerUniform = 0U;

/* Functional Uniforms */
GLuint keyPressedUniform = 0;

/* Data for Uniforms */
vmath::mat4 projectionMatrix = {};

GLfloat lightAmbient[]  = {0.3f, 0.3f, 0.3f, 3.0f};   // grey ambient light
GLfloat lightDiffused[] = {1.0f, 1.0f, 1.0f, 1.0f};   // white diffused light
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};   // specular color
GLfloat lightPosition[] = {10.0f, 0.0f, 10.0f, 1.0f}; // position of light

// boolean for lighting
BOOL bLightingEnabled  = FALSE;
BOOL bAnimationEnabled = FALSE;

GLfloat rotationAngle   = 0.0f;
GLuint  textureNormal   = 0U;
GLuint  textureDiffuse  = 0U;
GLuint  textureSpecular = 0U;

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
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdLine, int iCmdShow) // WinMain => --WinMainCRTStartup => --mainCRTStartup  (crt0.c)  CRT=? C Runtime 0th file  icmdshow => argc
{
    /* Local variables */
    WNDCLASSEX wndclass             = {0};  // window class typw of window , EX dor extended
    HWND       hwnd                 = NULL; // handle though it uint*  its uint only bcoz can't dereference it. opaque ptr
    MSG        msg                  = {0};  // struct
    TCHAR      szAppName[]          = TEXT("OpenGL Window Class Name");
    int        xDesktopWindowWidth  = 0;     // width of desktop window in pixels
    int        yDesktopWindowHeight = 0;     // height of desktop window in pixels
    int        x                    = 0;     // window leftmost point
    int        y                    = 0;     // window top point
    int        iResult              = 0;     // result of operations
    BOOL       bDone                = FALSE; // flag to indicate message loop compuletion status

    gpFILE = fopen("log.txt", "w");
    if (NULL == gpFILE)
    {
        MessageBox(NULL, TEXT("Log file can not be opened"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    fprintf(gpFILE, "Program for Black Screen started successfully!!!!\n");

    gpFile = fopen("abc.txt", "w");
    if (NULL == gpFile)
    {
        MessageBox(NULL, TEXT("Log file can not be opened"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    fprintf(gpFile, "Program for Black Screen started successfully!!!!\n");

    // WNDCLASSEX wndclass initialization predefined classes => button,dialog,scrollbar,status,dynamic
    wndclass.cbSize      = sizeof(WNDCLASSEX);                    // count of bytes in WNDCLASSEX
    wndclass.style       = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;    // redraw after resize (class style)
    wndclass.cbClsExtra  = 0;                                     // count of extra bytes in Class
    wndclass.cbWndExtra  = 0;                                     // count of extra bytes in window
    wndclass.lpfnWndProc = wndProc;                               // windows procedure callback function
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
        WS_EX_APPWINDOW, szAppName, TEXT("Rohit Nimkar RTR5"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, x, //(type of window name, Caption bar name, style of window => on
                                                                                                                                        // over all windows, where to show window onn desktop x,
        y, WIN_WIDTH, WIN_HEIGHT, NULL, // y,width,height set to defaults, whose child window is this default => os's child window or HWND_DESKTOP can be written
        NULL, hInstance, NULL);         // handle to menu, instance of whose window is to create, window msg after create so can be pass to wndproc through this param

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
                if (TRUE == bAnimationEnabled)
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

LRESULT CALLBACK wndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
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
                        fprintf(gpFILE, "F/f key pressed and entered into full screen successfully!!!!\n");
                        break;
                    }
                    else
                    {
                        ToggleFullScreen();
                        gbFullScreen = FALSE;
                        fprintf(gpFILE, "F/f key pressed and exited into full screen successfully!!!!\n");
                        break;
                    }
                    break;
                case 'L':
                case 'l':
                {
                    if (bLightingEnabled)
                    {
                        bLightingEnabled = FALSE;
                    }
                    else
                    {
                        bLightingEnabled = TRUE;
                    }
                }
                break;
                case 'A':
                case 'a':
                {
                    if (bAnimationEnabled)
                    {
                        bAnimationEnabled = FALSE;
                    }
                    else
                    {
                        bAnimationEnabled = TRUE;
                    }
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
    fprintf(gpFILE, "Hello\n\n");
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
        fprintf(gpFILE, "GetDC() failed\n");
        return -1;
    }

    /* Step 3: Request matching pixel format from windows */
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    // should return non-zero positive value on success
    if (0 == iPixelFormatIndex)
    {
        fprintf(gpFILE, "ChoosePizelFormat() failed\n");
        return -2;
    }

    /* Step 4: set obtained pixel format */
    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFILE, "SetPixelFormat() failed\n");
        return -3;
    }

    /* Step 5: Create opengl contexet from device context
               windows opengl bridging api => WGL windows graphics library
               tell WGL  to give me opengl compatible context from this dc
    */

    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL)
    {
        fprintf(gpFILE, "wglCreateContext() failed\n");
        return -4;
    }
    //(now ghdc will give up its control and gave it to ghrc)

    /* Step 6: make rendering context current */
    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        fprintf(gpFILE, "wglMakeCurrent() failed\n");
        return -5;
    }

    if (GLEW_OK != glewInit())
    {
        fprintf(gpFILE, "glewInit() failed\n");
        return (-6);
    }

    printGLInfo();

    fprintf(gpFILE, "Program compilation Successfull\n");
    /* Step 1:  Create vertex shader */
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    /* Step 2:  Attach source to shader */
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, nullptr);

    /* Step 3: Compile vertex shader */
    glCompileShader(vertexShaderObject);

    /* Step4: Check compilation errors */
    GLint   status        = 0;
    GLint   infoLogLength = 0;
    GLchar* szInfoLog     = nullptr;

    /* Step 4.1: Get compile status */
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);

    if (GL_FALSE == status)
    {
        /* Step 4.2: Get error log length */
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            szInfoLog = new char[infoLogLength + 1]();
            if (nullptr != szInfoLog)
            {
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, nullptr, szInfoLog);
                fprintf(gpFILE, "Vertex Shader compilation error log: %s\n", szInfoLog);
                delete[] szInfoLog;
                szInfoLog = nullptr;
            }
        }
        return (-7);
    }

    /* Step 1: Create fragment shader */
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    /* Step 2: Attach source to shader */
    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, nullptr);

    /* Step 3: Compile fragment shader */
    glCompileShader(fragmentShaderObject);

    /* Step 4.1: Get compile status */
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);

    if (GL_FALSE == status)
    {
        /* Step 4.2: Get error log length */
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            /* Step 4.3: allocate memory for log */
            szInfoLog = new char[infoLogLength + 1]();
            if (nullptr != szInfoLog)
            {
                /* Step 4.4: print error log */
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, nullptr, szInfoLog);
                fprintf(gpFILE, "Fragment Shader compilation error log: %s\n", szInfoLog);
                delete[] szInfoLog;
                szInfoLog = nullptr;
            }
        }
        return (-8);
    }

    fprintf(gpFILE, "Program compilation Successfull\n");
    /* Step 5: Create shader program */
    shaderProgramObject = glCreateProgram();

    /* Step 6: Attach shaders to programs */
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glAttachShader(shaderProgramObject, vertexShaderObject);

    /* Step 7: Bind attaribute location with shader program object */
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "aNormal");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "aTexCoord");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TANGENT, "aTangent");

    /* Step 8: Link shader program */
    glLinkProgram(shaderProgramObject);

    /* Step 9.1: Get program compilation status */
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
    if (GL_FALSE == status)
    {
        /* Step 9.2: Get error log length */
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            /* Step 9.3: allocate memory for log */
            szInfoLog = new char[infoLogLength + 1]();
            if (nullptr != szInfoLog)
            {
                /* Step 9.4: print error log */
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, nullptr, szInfoLog);
                fprintf(gpFILE, "Program linking error log: %s\n", szInfoLog);
                delete[] szInfoLog;
                szInfoLog = nullptr;
            }
        }
        return (-9);
    }

    // get uniform locations
    modelMatrixUniform      = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
    viewMatrixUniform       = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");

    viewPosUniform         = glGetUniformLocation(shaderProgramObject, "uViewPosition");
    lightPositionUniform   = glGetUniformLocation(shaderProgramObject, "uLightPosition");
    lightAmbientUniform    = glGetUniformLocation(shaderProgramObject, "uLightAmbient");
    lightDiffuseUniform    = glGetUniformLocation(shaderProgramObject, "uLightDiffused");
    lightSpecularUniform   = glGetUniformLocation(shaderProgramObject, "uLightSpecular");
    diffuseSamplerUniform  = glGetUniformLocation(shaderProgramObject, "uDiffuseSampler");
    specularSamplerUniform = glGetUniformLocation(shaderProgramObject, "uSpecularSampler");
    normalSamplerUniform   = glGetUniformLocation(shaderProgramObject, "uNormalSampler");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vboPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vboNormal);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vboTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vboTangent);
    glBindBuffer(GL_ARRAY_BUFFER, vboTangent);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tangents), tangents, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TANGENT, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TANGENT);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &eboSquare);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSquare);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    projectionMatrix = vmath::mat4::identity();

    textureDiffuse  = loadTexture("textures/cracked_concrete_wall_diff_1k.png");
    textureSpecular = loadTexture("textures/cracked_concrete_wall_rough_1k.png");
    textureNormal   = loadTexture("textures/cracked_concrete_wall_nor_gl_1k.png");

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    resize(WIN_WIDTH, WIN_HEIGHT);
    return 0;
}

void printGLInfo(void)
{
    GLint nExtensions = 0;
    GLint idx         = 0;

    fprintf(gpFILE, "OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
    fprintf(gpFILE, "OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    fprintf(gpFILE, "OpenGL Version: %s\n", glGetString(GL_VERSION));
    fprintf(gpFILE, "OpenGL SL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    /* List supported extensions */
    // glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    // for (idx = 0; idx < nExtensions; ++idx) { fprintf(gpFILE, "%s\n", glGetStringi(GL_EXTENSIONS, idx)); }
}

void resize(int width, int height)
{
    if (height <= 0)
        height = 1;

    projectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    glViewport(0, 0, width, height);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vmath::mat4 modelMatrix       = vmath::mat4::identity();
    vmath::mat4 translationMatrix = vmath::mat4::identity();
    vmath::mat4 scaleMatrix       = vmath::mat4::identity();
    vmath::mat4 viewMatrix        = vmath::mat4::identity();
    vmath::mat4 rotationMatrix    = vmath::mat4::identity();

    glUseProgram(shaderProgramObject);
    glBindVertexArray(vao);

    translationMatrix = vmath::translate(0.0f, 0.0f, 0.0f);
    scaleMatrix       = vmath::scale(2.0f, 2.0f, 2.0f);

    viewMatrix  = vmath::lookat(vmath::vec3(0.0f, 0.0f, 5.0f), vmath::vec3(0.0f, 0.0f, -1.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

    glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, projectionMatrix);
    glUniform3fv(viewPosUniform, 1, vmath::vec3(0.0f, 0.0f, 5.0f));

    glUniform3fv(lightAmbientUniform, 1, lightAmbient);
    glUniform3fv(lightDiffuseUniform, 1, lightDiffused);
    glUniform3fv(lightSpecularUniform, 1, lightSpecular);
    glUniform3fv(lightPositionUniform, 1, lightPosition);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureDiffuse);
    glUniform1i(diffuseSamplerUniform, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureNormal);
    glUniform1i(normalSamplerUniform, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureSpecular);
    glUniform1i(specularSamplerUniform, 2);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0U);

    glUseProgram(0);
    glBindVertexArray(0);
}

void update(void)
{
    rotationAngle = rotationAngle + 0.0005;
    if (360.0f < rotationAngle)
    {
        rotationAngle = rotationAngle - 360.0f;
    }

    lightPosition[0] = 10.0f * cosf(rotationAngle);
    lightPosition[1] = 0.0f;
    lightPosition[2] = 10.0f * sinf(rotationAngle);
}

void uninitialize(void)
{
    if (0U != shaderProgramObject)
    {
        glUseProgram(shaderProgramObject);
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

    // if (0U != textureDiffuse)
    // {
    //     // glBindTexture(GL_TEXTURE_2D, textureDiffuse);
    //     fprintf(gpFILE, "Deleting texture %d\n", textureDiffuse);
    //     // glDeleteTextures(1, &textureDiffuse);
    //     // textureDiffuse = 0U;
    //     // glBindTexture(GL_TEXTURE_2D, textureDiffuse);
    // }

    // if (0U != textureNormal)
    // {
    //     // glBindTexture(GL_TEXTURE_2D, textureNormal);
    //     fprintf(gpFILE, "Deleting texture %d\n", textureNormal);
    //     // glDeleteTextures(1, &textureNormal);
    //     // textureNormal = 0U;
    //     // glBindTexture(GL_TEXTURE_2D, textureNormal);
    // }

    // if (0U != textureSpecular)
    // {
    //     // glBindTexture(GL_TEXTURE_2D, textureSpecular);
    //     fprintf(gpFILE, "Deleting texture %d\n", textureSpecular);
    //     // glDeleteTextures(1, &textureSpecular);
    //     // textureSpecular = 0U;
    //     // glBindTexture(GL_TEXTURE_2D, textureSpecular);
    // }

    if (0U != vboPosition)
    {
        glDeleteBuffers(1, &vboPosition);
        vboPosition = 0U;
    }
    if (0U != eboSquare)
    {
        glDeleteBuffers(1, &eboSquare);
        eboSquare = 0U;
    }

    if (0U != vboTexCoords)
    {
        glDeleteBuffers(1, &vboTexCoords);
        vboTexCoords = 0U;
    }

    if (0U != vboNormal)
    {
        glDeleteBuffers(1, &vboNormal);
        vboNormal = 0U;
    }

    if (0U != vboTangent)
    {
        glDeleteBuffers(1, &vboTangent);
        vboTangent = 0U;
    }

    if (0U != vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0U;
    }

    if (gbFullScreen == TRUE)
    {
        ToggleFullScreen();
        gbFullScreen = FALSE;
    }

    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(nullptr, nullptr);
    }

    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = nullptr;
    }

    if (ghdc)
    {
        ReleaseDC(gHwnd, ghdc);
        ghdc = nullptr;
    }

    if (gHwnd)
    {
        DestroyWindow(gHwnd);
        gHwnd = nullptr;
    }

    // if (NULL != gpFILE)
    {
        fflush(gpFILE);
        fprintf(gpFILE, "Program for base code terminated successfully!!!!\n");
        fclose(gpFILE);
        gpFILE = nullptr;
    }
    fprintf(gpFile, "Program terminated successfully!!!!\n");
    fclose(gpFile);
}

unsigned int loadTexture(const char* filename)
{
    unsigned char* pixel_data = NULL;
    int            width, height, nrComponents;
    GLenum         format;
    unsigned int   textureId;

    pixel_data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if (pixel_data != NULL)
    {
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set up texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        // push the data to texture memory
        glTexImage2D(GL_TEXTURE_2D, 0, format, (GLint)width, (GLint)height, 0, format, GL_UNSIGNED_BYTE, (const void*)pixel_data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(pixel_data);
        pixel_data = NULL;
    }
    else
    {
        fprintf(gpFILE, "Error : failed to load texture %s.\n", filename);
    }
    return textureId;
}