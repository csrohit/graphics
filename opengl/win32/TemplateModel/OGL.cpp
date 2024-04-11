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
#include "load.h"

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
HWND            gHwnd        = NULL; // global window handle
BOOL            gbActive     = FALSE;
DWORD           gdwStyle     = 0;                         // unsigned long 32bit
WINDOWPLACEMENT gwpPrev      = {sizeof(WINDOWPLACEMENT)}; // previous window placement
BOOL            gbFullScreen = FALSE;                     // win32s BOOL not c++ bool
HDC             ghdc         = NULL;                      // global handle for device context
HGLRC           ghrc         = NULL;                      // Global handle for rendering context

struct Model* pModel = NULL;

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
    fprintf(gpFILE, "Program for Model Loading started successfully!!!!\n");


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

    int a = loadModel("./rock.obj", &pModel);
    printModel(pModel);

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
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
    for (idx = 0; idx < nExtensions; ++idx) { fprintf(gpFILE, "%s\n", glGetStringi(GL_EXTENSIONS, idx)); }
}

void resize(int width, int height)
{
    if (height <= 0)
        height = 1;

    // projectionMatrix = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    glViewport(0, 0, width, height);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void update(void)
{

}

void uninitialize(void)
{
    unloadModel(pModel);

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

    if (NULL != gpFILE)
    {
        fflush(gpFILE);
        fprintf(gpFILE, "Program for base code terminated successfully!!!!\n");
        fclose(gpFILE);
        gpFILE = nullptr;
    }
}
