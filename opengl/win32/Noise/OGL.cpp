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
#include "Sphere.h"
#include "noise.h"

/* Macro definitions */
#define WIN_WIDTH     800
#define WIN_HEIGHT    600
#define UNUSED_VAL(x) ((void)x)

/* Link with OpenGl libraries */
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "Sphere.lib")

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_UVS,
    AMC_ATTRIBUTE_NORMALS
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
void resize(int width, int height);
void loadNoiseTexture(void);

void printGLInfo(void);

/* Global variable declaration */
FILE*           gpFILE       = NULL;
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
    "in      vec4 aPosition;"
    "in      vec3 aNormal;"
    "uniform mat4 uModelViewMatrix;"
    "uniform mat4 uProjectionMatrix;"
    "uniform vec3 uld;"
    "uniform vec3 ukd;"
    "uniform vec4 uLightPosition;"
    "uniform int  uKeyPressed;"
    "out     vec3 oDiffuseLight;"
    "out     vec3 oTexCoord;"
    "void main(void)"
    "{"
    "    if(uKeyPressed == 1)"
    "    {"
    "        vec4 eyePosition  = uModelViewMatrix * aPosition;"
    "        mat3 normalMatrix = mat3(transpose(inverse(uModelViewMatrix)));"
    "        vec3 n            = normalize(normalMatrix * aNormal );"
    "        vec3 s            = normalize(vec3(uLightPosition - eyePosition));"
    "        oDiffuseLight     = uld * ukd * max(dot(s,n),0.0); "
    "    }"
    "    else"
    "    {"
    "        oDiffuseLight = vec3(0.0, 0.0, 0.0);"
    "    }"
    "    gl_Position = uProjectionMatrix * uModelViewMatrix * aPosition;"
    "    oTexCoord = aPosition.xyz;"
    "}";

GLuint vertexShaderObject = 0U;

const GLchar* fragmentShaderSourceCode =
    "#version 460 core"
    "\n"
    "in      vec3 oDiffuseLight;"
    "uniform int  uKeyPressed;"
    "out     vec4 FragColor;"
    "in vec3 oTexCoord;"
    "uniform sampler3D uSamplerNoise;"
    "uniform vec3 uColorCloud;"
    "uniform vec3 uColorSky;"
    "void main(void)"
    "{"
    "    vec4 noiseColor = texture(uSamplerNoise, oTexCoord);"
    "    float intensity = (noiseColor.x + noiseColor.y + noiseColor.z)/1.0f; "
    "    vec3 f = mix(uColorSky, uColorCloud, intensity);"
    "    FragColor = vec4(f,1.0);"
    "}";

GLuint fragmentShaderObject = 0U;

GLuint shaderProgramObject = 0U;

// clang-format off
GLfloat positions[1146] = {};
GLfloat normals[1146] = {};
GLfloat textures[764] = {};
unsigned short elements[2280] = {};
unsigned int gnPositions = 0U;
unsigned int gnElements = 0U;

// clang-format on

GLuint vao         = 0U;
GLuint vboPosition = 0U;
GLuint vboNormal   = 0U;
GLuint eboSpheres  = 0U;

vmath::mat4 perspectiveProjectionMatrix = {};

GLuint modelViewMatrixUniform  = 0;
GLuint projectionMatrixUniform = 0;
GLuint ldUniform               = 0;
GLuint kdUniform               = 0;
GLuint lightPositionUniform    = 0;
GLuint keyPressedUniform       = 0;

// boolean for lighting
BOOL bLightingEnabled  = FALSE;
BOOL bAnimationEnabled = FALSE;

GLfloat lightDiffuse[]    = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialDiffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat lightPosition[]   = {0.0f, 0.0f, 2.0f, 1.0f};

GLuint textureNoise;
GLuint noiseSamplerUniform;
GLuint colorSkyUniform;
GLuint colorCloudUniform;

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

    gpFILE = fopen("Log.txt", "w");
    if (NULL == gpFILE)
    {
        MessageBox(NULL, TEXT("Log file can not be opened"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
        exit(0);
    }
    fprintf(gpFILE, "Program for Black Screen started successfully!!!!\n");

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
                bDone = TRUE;
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
    PIXELFORMATDESCRIPTOR pfd               = {};
    int                   iPixelFormatIndex = 0;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    getSphereVertexData(positions, normals, textures, elements);
    gnPositions = getNumberOfSphereVertices();
    gnElements  = getNumberOfSphereElements();

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

    /* Step 5: Create shader program */
    shaderProgramObject = glCreateProgram();

    /* Step 6: Attach shaders to programs */
    glAttachShader(shaderProgramObject, fragmentShaderObject);
    glAttachShader(shaderProgramObject, vertexShaderObject);

    /* Step 7: Bind attaribute location with shader program object */
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMALS, "aNormal");

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
    fprintf(gpFILE, "Program compilation Successfull\n");

    modelViewMatrixUniform  = glGetUniformLocation(shaderProgramObject, "uModelViewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
    ldUniform               = glGetUniformLocation(shaderProgramObject, "uld");
    kdUniform               = glGetUniformLocation(shaderProgramObject, "ukd");
    lightPositionUniform    = glGetUniformLocation(shaderProgramObject, "uLightPosition");
    keyPressedUniform       = glGetUniformLocation(shaderProgramObject, "uKeyPressed");

    colorCloudUniform = glGetUniformLocation(shaderProgramObject, "uColorCloud");
    colorSkyUniform   = glGetUniformLocation(shaderProgramObject, "uColorSky");
    noiseSamplerUniform   = glGetUniformLocation(shaderProgramObject, "uSamplerNoise");

    /* Cube */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPosition);
    glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    glGenBuffers(1, &vboNormal);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMALS, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMALS);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    glGenBuffers(1, &eboSpheres);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSpheres);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    glBindVertexArray(0U);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    loadNoiseTexture();
    /* Enabling Depth */
    glClearDepth(1.0f);      //[Compulsory] Make all bits in depth buffer as '1'
    glEnable(GL_DEPTH_TEST); //[Compulsory] enable depth test
    glDepthFunc(GL_LEQUAL);  //[Compulsory] Which function to use for testing
    perspectiveProjectionMatrix = vmath::mat4::identity();
    /* Drawing control is now with OpenGL */
    // set the clear color of window to blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // ranges from 0.0 to 1.0 => 2^32 (pfd.cColorBits = 32) not make screen blue , its just tell to use bluee brush/clr
    resize(WIN_WIDTH, WIN_HEIGHT);
    return 0;
}

void loadNoiseTexture(void)
{
    glGenTextures(1, &textureNoise);
    glBindTexture(GL_TEXTURE_3D, textureNoise);
    init3DNoiseTexture();
    glBindTexture(GL_TEXTURE_3D, 0);
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
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    for (idx = 0; idx < nExtensions; ++idx) { fprintf(gpFILE, "%s\n", glGetStringi(GL_EXTENSIONS, idx)); }
}

void resize(int width, int height)
{
    if (height <= 0)
        height = 1;

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    glViewport(0, 0, width, height);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window with color whose bit is set, all bits in depth buffer set to 1 (if value is 1 or lower because of LEQUAL)

    vmath::mat4 modelViewMatrix   = vmath::mat4::identity();
    vmath::mat4 translationMatrix = vmath::mat4::identity();

    glUseProgram(shaderProgramObject);
    /* cube */

    glBindVertexArray(vao);
    translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);

    vmath::mat4 scaleMatrix = vmath::mat4::identity();
    modelViewMatrix         = translationMatrix;

    glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, modelViewMatrix);

    glUniform3f(colorSkyUniform, 1.0f, 1.0f, 1.0f);
    glUniform3f(colorCloudUniform, 0.0f, 0.0f, 1.0f);

    glActiveTexture(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, textureNoise);
    glUniform1i(noiseSamplerUniform, 0);

    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);
    if (bLightingEnabled == TRUE)
    {
        glUniform1i(keyPressedUniform, 1);
        glUniform3fv(ldUniform, 1, lightDiffuse);
        glUniform3fv(kdUniform, 1, materialDiffuse);
        glUniform4fv(lightPositionUniform, 1, lightPosition);
    }
    else
    {
        glUniform1i(keyPressedUniform, 0);
    }

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
    glDrawElements(GL_TRIANGLES, gnElements, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0U);

    SwapBuffers(ghdc); // as in pfd.dwFlags = PFD_DOUBLEBUFFER
    // ghrc is for=> opengl & ghdc is for=> OS
}

void update(void)
{
}

void uninitialize(void)
{
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

    if (0U != vboNormal)
    {
        glDeleteBuffers(1, &vboNormal);
        vboNormal = 0U;
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

    /* reset rendering context */
    if (wglGetCurrentContext() == ghrc) // if current context is ghrc  then make current context as NULL
    {
        wglMakeCurrent(nullptr, nullptr); // make current context NULL
    }

    /* Delete OpenGL rendering context */
    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = nullptr;
    }

    /* release global dc */
    if (ghdc)
    {
        ReleaseDC(gHwnd, ghdc); // gHwnd => whose windows ghdc =>which ghdc
        ghdc = nullptr;
    }

    /* Destroy window */
    if (gHwnd)
    {
        DestroyWindow(gHwnd);
        gHwnd = nullptr;
    }

    /* Close logging file */
    if (gpFILE)
    {
        fprintf(gpFILE, "Program for base code terminated successfully!!!!\n");
        fclose(gpFILE);
        gpFILE = nullptr;
    }
}
