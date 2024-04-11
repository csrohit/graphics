/**
 * @file    ogl.cpp
 * @brief   Color Triangle
 * @author  Rohit Nimkar
 * @date    28/01/2024
 * @version 1.1
 */

/* Windows Header files */
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

/* OpenGL header files */
#include <gl/glew.h> // this must be before gl/GL.h
#include <gl/GL.h>
#include "ogl.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"

/* Macro definitions */
#define WIN_WIDTH     800
#define WIN_HEIGHT    600
#define UNUSED_VAL(x) ((void)x)

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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
void displayImGui(void);

/**
 * @brief Update contents on screen
 */
void update(void);

/**
 * @brief toggle  full screen
 */
void ToggleFullScreen(void);

void IMGUI_initialize(void);

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
    "in  vec4 aPosition;"
    "in  vec4 aColor;"
    "out vec4 oColor;"
    "void main(void)"
    "{"
    "    gl_Position = aPosition;"
    "    oColor = aColor;"
    "}";

GLuint vertexShaderObject = 0U;

const GLchar* fragmentShaderSourceCode =
    "#version 460 core"
    "\n"
    "in vec4 oColor;"
    "out vec4 FragColor;"
    "void main(void)"
    "{"
    "FragColor=oColor;"
    "}";

GLuint fragmentShaderObject = 0U;

GLuint shaderProgramObject = 0U;

// clang-format off
const GLfloat positions[] = 
{
     0.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f
};

const GLfloat colors[] = 
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};
// clang-format on

GLuint vao          = 0U;
GLuint vbo_position = 0U;
GLuint vbo_color    = 0U;

/* ImGUI variables */
GLfloat topVertexColor[]         = {1.0f, 0.0f, 0.0f};
GLfloat bottomLeftVertexColor[]  = {0.0f, 1.0f, 0.0f};
GLfloat bottomRightVertexColor[] = {0.0f, 0.0f, 1.0f};

GLfloat triangleColor[3][3] = {};

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

    IMGUI_initialize();

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
                displayImGui();
                display();
                update();
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                
                SwapBuffers(ghdc); // as in pfd.dwFlags = PFD_DOUBLEBUFFER
            }
        }
    }

    /* clean up */
    uninitialize();

    return (int)msg.wParam;
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    if(ImGui_ImplWin32_WndProcHandler(hwnd, iMsg, wParam, lParam))
    {
        return true;
    }
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

void IMGUI_initialize(void)
{
    IMGUI_CHECKVERSION();

    ImGuiContext* ctx = ImGui::CreateContext();
    if (NULL == ctx)
    {
        fprintf(gpFILE, "IMGUI initialize failed");
        uninitialize();
    }

    if (false == ImGui_ImplWin32_InitForOpenGL(gHwnd))
    {
        fprintf(gpFILE, "IMGUI Win32 Init for opengl failed");
        uninitialize();
    }

    if (false == ImGui_ImplOpenGL3_Init("#version 460"))
    {
        fprintf(gpFILE, "IMGUI OpenGl3 Init failed");
        uninitialize();
    }
}

int initialize(void)
{
    PIXELFORMATDESCRIPTOR pfd               = {};
    int                   iPixelFormatIndex = 0;
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    // initialization of pixelformat descriptor
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

    ghdc = GetDC(gHwnd);
    if (ghdc == NULL)
    {
        fprintf(gpFILE, "GetDC() failed\n");
        return -1;
    }

    /* step 3 */
    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    // should return non-zero positive value on success
    if (0 == iPixelFormatIndex)
    {
        fprintf(gpFILE, "ChoosePizelFormat() failed\n");
        return -2;
    }

    /* Step 4 : set obtained pixel format */
    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        fprintf(gpFILE, "SetPixelFormat() failed\n");
        return -3;
    }

    /* step 5 : create opengl contexet from device context */
    // windows opengl bridging api => WGL windows graphics library
    // tell WGL  to give me opengl compatible dc
    // tell WGL  to give me opengl compatible context from this dc

    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL)
    {
        fprintf(gpFILE, "wglCreateContext() failed\n");
        return -4;
    }
    //(now ghdc will give up its control and gave it to ghrc)

    /* step6: make rendering context current */
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

    /*Step 1:  Create vertex shader */
    vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

    /*Step 2:  Attach source to shader */
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, nullptr);

    /*Step 3: Compile vertex shader */
    glCompileShader(vertexShaderObject);

    /*Step4: Check compilation errors */
    GLint   status        = 0;
    GLint   infoLogLength = 0;
    GLchar* szInfoLog     = nullptr;

    /*Step 4.1: Get compile status */
    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);

    if (GL_FALSE == status)
    {
        /*Step 4.2: Get error log length*/
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

    /*Step 1:  Create fragment shader */
    fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

    /*Step 2:  Attach source to shader */
    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, nullptr);

    /*Step 3: Compile fragment shader */
    glCompileShader(fragmentShaderObject);

    /*Step 4.1: Get compile status */
    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);

    if (GL_FALSE == status)
    {
        /*Step 4.2: Get error log length*/
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            /*Step 4.3: allocate memory for log*/
            szInfoLog = new char[infoLogLength + 1]();
            if (nullptr != szInfoLog)
            {
                /*Step 4.4: print error log*/
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
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "aColor");

    /* Step 8: Link shader program */
    glLinkProgram(shaderProgramObject);

    /* Step 9: Get program compilation status */
    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
    if (GL_FALSE == status)
    {
        /*Step 4.2: Get error log length*/
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (0 != infoLogLength)
        {
            /*Step 4.3: allocate memory for log*/
            szInfoLog = new char[infoLogLength + 1]();
            if (nullptr != szInfoLog)
            {
                /*Step 4.4: print error log*/
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, nullptr, szInfoLog);
                fprintf(gpFILE, "Program linking error log: %s\n", szInfoLog);
                delete[] szInfoLog;
                szInfoLog = nullptr;
            }
        }
        return (-9);
    }
    fprintf(gpFILE, "Program compilation Successfull\n");

    /* VAO */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* VBO for position */
    glGenBuffers(1, &vbo_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    /* VBO for color */
    glGenBuffers(1, &vbo_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColor), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0U);

    glBindVertexArray(0U);

    /* Enabling Depth */
    glClearDepth(1.0f);      //[Compulsory] Make all bits in depth buffer as '1'
    glEnable(GL_DEPTH_TEST); //[Compulsory] enable depth test
    glDepthFunc(GL_LEQUAL);  //[Compulsory] Which function to use for testing

    /* Drawing control is now with OpenGL */
    // set the clear color of window to blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // ranges from 0.0 to 1.0 => 2^32 (pfd.cColorBits = 32) not make screen blue , its just tell to use bluee brush/clr

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

    glMatrixMode(GL_PROJECTION);                       // for matrix calculation while resizing use GL_PROJECTION
    glLoadIdentity();                                  // take identity matrix for beginning
    glViewport(0, 0, (GLsizei)width, (GLsizei)height); // bioscope/Binoculor => focus on which are to be see in window => here we telling to focus on whole window

    GLdouble gldHeight = (GLdouble)(tan(M_PI / 8.0f) * 0.1);
    GLdouble gldWidth  = gldHeight * ((GLdouble)width / (GLdouble)height);

    /* after setting projection matrix, ensure that VIEW matrix is also set */
    glFrustum(-gldWidth, gldWidth, -gldHeight, gldHeight, 0.1, 100.0);
}

void displayImGui(void)
{
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Template");
    {
        ImGui::SliderFloat3("Top Vertex Color", topVertexColor, 0.0f, 1.0f);
        ImGui::SliderFloat3("Left Bottom Vertex Color", bottomLeftVertexColor, 0.0f, 1.0f);
        ImGui::SliderFloat3("Right Bottom Vertex Color", bottomRightVertexColor, 0.0f, 1.0f);
    }
    ImGui::End();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window with color whose bit is set, all bits in depth buffer set to 1 (if value is 1 or lower because of LEQUAL)

    glUseProgram(shaderProgramObject);
    glBindVertexArray(vao);

    triangleColor[0][0] = topVertexColor[0];
    triangleColor[0][1] = topVertexColor[1];
    triangleColor[0][2] = topVertexColor[2];

    triangleColor[1][0] = bottomLeftVertexColor[0];
    triangleColor[1][1] = bottomLeftVertexColor[1];
    triangleColor[1][2] = bottomLeftVertexColor[2];

    triangleColor[2][0] = bottomRightVertexColor[0];
    triangleColor[2][1] = bottomRightVertexColor[1];
    triangleColor[2][2] = bottomRightVertexColor[2];

    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColor), triangleColor, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0U);
}

void update(void)
{
    /* Update states */
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

    if (0U != vbo_position)
    {
        glDeleteBuffers(1, &vbo_position);
        vbo_position = 0U;
    }

    if (0U != vbo_color)
    {
        glDeleteBuffers(1, &vbo_color);
        vbo_color = 0U;
    }

    if (0U != vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0U;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

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
