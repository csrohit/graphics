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
#include "stb_image.h"

/* Macro definitions */
#define WIN_WIDTH     800
#define WIN_HEIGHT    600
#define UNUSED_VAL(x) ((void)x)
#define INCREMENT     0.1f

/* Link with OpenGl libraries */
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "OpenGL32.lib")

enum
{
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR
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

BOOL         compileShader(unsigned int shaderId, const char* shaderSourceCode);
BOOL         linkProgram(GLuint programId);
unsigned int loadShaders(const char* vertexShaderSourceCode, const char* fragmentShaderSourceCode);
unsigned int loadCubemap(char** faces);

/**
 * @brief Resize window
 *
 * @param width  [in] - requested width
 * @param height [in] - Requested height
 */
void resize(int width, int height);

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
    "in vec4 aPosition;"
    "in vec4 aColor;"
    "out vec4 oColor;"
    "uniform mat4 uMVPMatrix;"
    "void main(void)"
    "{"
    "gl_Position =uMVPMatrix * aPosition;"
    "oColor = aColor;"
    "}";

const GLchar* fragmentShaderSourceCode =
    "#version 460 core"
    "\n"
    "in vec4 oColor;"
    "out vec4 FragColor;"
    "void main(void)"
    "{"
    "FragColor=oColor;"
    "}";

const GLchar* skyoxVertexShaderSourceCode =
    "#version 460 core"
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

const GLchar* skyboxFragmentShaderSourceCode =
    "#version 460 core"
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

// clang-format off

const GLfloat modelPositions[] = 
{
    /* Front */
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,

    /* Top */
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,

    /* Bottom */
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,

    /* Back */
    1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    /* Right */
    1.0f, 1.0f, -1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f,

    /* Left */
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, 1.0f,
};

float skyboxVertices[] =
{
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

// clang-format on

GLuint vaoModel          = 0U;
GLuint vboModelPositions = 0U;

GLuint vaoSkybox          = 0U;
GLuint vboSkyboxPOsitions = 0U;
GLuint eboSkyboxIndices   = 0U;

vmath::mat4 projectionMatrix       = {};
GLuint      mvpMatrixUniform       = 0U;
GLuint      mvpMatrixSkyboxUniform = 0U;
GLuint      textureSamplerUniform  = 0U;
GLuint      textureSmiley          = 0U;

GLfloat anglecube = 0.0f;
GLfloat eyeAngle  = 0.0f;
GLfloat radius    = 5.0f;

vmath::vec3 eyePosition = vmath::vec3(0.0f, 0.0f, 2.0f);
vmath::vec3 eyeTarget   = vmath::vec3(0.0f, 0.0f, 0.0f);

const char* arr[] = {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};

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
                SwapBuffers(ghdc);
                update();
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
                case 'x': eyePosition[0] = eyePosition[0] + INCREMENT; break;
                case 'X': eyePosition[0] = eyePosition[0] - INCREMENT;
                case 'y': eyePosition[1] = eyePosition[1] + INCREMENT; break;
                case 'Y': eyePosition[1] = eyePosition[1] - INCREMENT;
                case 'z': eyePosition[2] = eyePosition[2] + INCREMENT; break;
                case 'Z': eyePosition[2] = eyePosition[2] - INCREMENT; break;

                case 'a': eyeTarget[0] = eyeTarget[0] + INCREMENT; break;
                case 'A': eyeTarget[0] = eyeTarget[0] - INCREMENT;
                case 's': eyeTarget[1] = eyeTarget[1] + INCREMENT; break;
                case 'S': eyeTarget[1] = eyeTarget[1] - INCREMENT;
                case 'd': eyeTarget[2] = eyeTarget[2] + INCREMENT; break;
                case 'D': eyeTarget[2] = eyeTarget[2] - INCREMENT; break;
                case 'b':
                {
                    radius = radius + INCREMENT;
                    eyePosition[0] = radius * cosf(eyeAngle);
                    eyePosition[2] = radius * sinf(eyeAngle);
                    break;
                }
                case 'B':
                {
                    radius = radius - INCREMENT;
                    eyePosition[0] = radius * cosf(eyeAngle);
                    eyePosition[2] = radius * sinf(eyeAngle);
                    break;
                }
                case 'v':
                {
                    eyeAngle += INCREMENT;
                    eyePosition[0] = radius * cosf(eyeAngle);
                    eyePosition[2] = radius * sinf(eyeAngle);
                    break;
                }
                case 'V':
                {
                    eyeAngle -= INCREMENT;
                    eyePosition[0] = radius * cosf(eyeAngle);
                    eyePosition[2] = radius * sinf(eyeAngle);
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

    shaderProgramObject = loadShaders(vertexShaderSourceCode, fragmentShaderSourceCode);
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "aPosition");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "aColor");
    linkProgram(shaderProgramObject);
    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");

    skyboxShaderProgramId = loadShaders(skyoxVertexShaderSourceCode, skyboxFragmentShaderSourceCode);
    glBindAttribLocation(skyboxShaderProgramId, AMC_ATTRIBUTE_POSITION, "aPosition");
    linkProgram(skyboxShaderProgramId);
    mvpMatrixSkyboxUniform = glGetUniformLocation(shaderProgramObject, "uMVPMatrix");
    textureSamplerUniform  = glGetUniformLocation(shaderProgramObject, "skybox");

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
    glGenBuffers(1, &vboSkyboxPOsitions);
    glBindBuffer(GL_ARRAY_BUFFER, vboSkyboxPOsitions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    glGenBuffers(1, &eboSkyboxIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboSkyboxIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);
    glBindVertexArray(0U);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    textureSmiley = loadCubemap((char**)arr);

    /* Enabling Depth */
    glClearDepth(1.0f);      //[Compulsory] Make all bits in depth buffer as '1'
    glEnable(GL_DEPTH_TEST); //[Compulsory] enable depth test
    glDepthFunc(GL_LESS);    //[Compulsory] Which function to use for testing

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    for (idx = 0; idx < nExtensions; ++idx) { fprintf(gpFILE, "%s\n", glGetStringi(GL_EXTENSIONS, idx)); }
}

void resize(int width, int height)
{
    if (height <= 0)
        height = 1;
    glViewport(0, 0, width, height);

    projectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window with color whose bit is set, all bits in depth buffer set to 1 (if value is 1 or lower because of LEQUAL)

    vmath::mat4 modelViewProjectionMatrix = vmath::mat4::identity();
    vmath::mat4 translationMatrix         = vmath::translate(0.0f, 0.0f, -6.0f);
    vmath::mat4 scaleMatrix               = vmath::mat4::identity();
    vmath::mat4 viewMatrix                = vmath::mat4::identity();
    vmath::mat4 rotationMatrix            = vmath::rotate(anglecube, 0.0f, 1.0f, 0.0f);

    /* cube */
    glUseProgram(shaderProgramObject);
    glBindVertexArray(vaoModel);
    {
        viewMatrix = vmath::lookat(eyePosition, eyeTarget, vmath::vec3(0.0f, 1.0f, 0.0f));

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
        scaleMatrix               = vmath::scale(13.0f, 13.0f, 13.0f);
        modelViewProjectionMatrix = projectionMatrix * viewMatrix * rotationMatrix * scaleMatrix;
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureSmiley);
        glUniform1i(textureSamplerUniform, 0);
        
        glDepthFunc(GL_LEQUAL);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LESS);
    }
    glBindVertexArray(0U);
    glUseProgram(0);
}

void update(void)
{
    anglecube = anglecube + 0.01;
    if (360.0f < anglecube)
    {
        anglecube = anglecube - 360.0f;
    }
}

void uninitialize(void)
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

unsigned int loadShaders(const char* vertexShaderSourceCode, const char* fragmentShaderSourceCode)
{
    BOOL   status    = GL_FALSE;
    GLuint programId = 0U;

    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);

    if (GL_TRUE == compileShader(vertexShaderId, vertexShaderSourceCode))
    {
        GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        if (GL_TRUE == compileShader(fragmentShaderId, fragmentShaderSourceCode))
        {
            unsigned int programId = glCreateProgram();
            glAttachShader(programId, vertexShaderId);
            glAttachShader(programId, fragmentShaderId);
            return programId;
        }
    }
    return programId;
}

BOOL compileShader(unsigned int shaderId, const char* shaderSourceCode)
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
                fprintf(gpFILE, "Vertex Shader compilation error log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = nullptr;
            }
        }
    }
    return status;
}

BOOL linkProgram(GLuint programId)
{
    GLint status = GL_FALSE;
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
                fprintf(gpFILE, "Program linking error log: %s\n", szInfoLog);
                free(szInfoLog);
                szInfoLog = nullptr;
            }
        }

        glUseProgram(programId);
        GLint nShaders = 0;
        glGetProgramiv(programId, GL_ATTACHED_SHADERS, &nShaders);
        if (0 < nShaders)
        {
            GLuint* pShaders = new GLuint[nShaders]();
            if (nullptr != pShaders)
            {
                glGetAttachedShaders(programId, nShaders, nullptr, pShaders);
                for (GLuint idx = 0U; idx <= nShaders; ++idx)
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
        glDeleteProgram(programId);
    }
    return status;
}

unsigned int loadCubemap(char** faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char* data = stbi_load(faces[i], &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            fprintf(gpFILE, "Cubemap texture failed to load at path: %s", faces[i]);
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