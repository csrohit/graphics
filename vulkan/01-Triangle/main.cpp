/**
 * @file      main.cpp
 * @brief     Vulkan Triangle
 * @author    Rohit Nimkar
 * @version   1.0
 * @date      2023-12-29
 * @copyright Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */

/*--- System headers ---*/
#include <vector>
#define _USE_MATH_DEFINES
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <math.h>

/*--- Xlib headers ---*/
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/*--- Vulkan headers ---*/
#include "myheader.h"
/*--- Defines ---*/
#define WIN_WIDTH  800
#define WIN_HEIGHT 600

/* function declaration */
static int  initialize();
static void uninitialize();
static void display();
static void update();
static void resize(int width, int height);
static void toggleFullscreen(Display *display, Window window);

/* Windowing related variables */
Display    *dpy          = nullptr; // connection to server
Window      w            = 0UL;     // handle of current window
bool        gbAbortFlag  = false;   // Global abort flag
XRectangle  rect         = {0};     // window dimentions rectangle
bool        gbFullscreen = false;   // should display in fullscreen mode
bool        shouldDraw   = false;   // should scene be rendered
Colormap    colormap;
XVisualInfo visualInfo;

/*--- Entry point --*/
int main(int argc, char *argv[])
{
    static Atom wm_delete_window = 0;   // atomic variable to detect close button click
    Window      root             = 0UL; // handle of root window [Desktop]
    dpy                          = XOpenDisplay(nullptr);

    if (dpy == nullptr)
    {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(dpy);
    root       = XDefaultRootWindow(dpy);

    Status status = XMatchVisualInfo(dpy, screen, DefaultDepth(dpy, screen), TrueColor, &visualInfo);
    if (status == 0)
    {
        printf("XMatchVisualInfo failed\n");
        uninitialize();
        exit(1);
    }
    XSetWindowAttributes xattr;
    xattr.event_mask        = ExposureMask | KeyPressMask;
    xattr.border_pixel      = BlackPixel(dpy, screen);
    xattr.background_pixel  = WhitePixel(dpy, screen);
    xattr.override_redirect = true;
    xattr.colormap          = XCreateColormap(dpy, root, visualInfo.visual, AllocNone);
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask;
    colormap                = xattr.colormap;

    w = XCreateWindow(dpy,                      // pointer to display
                      root,                     // handle to root window
                      0,                        // left
                      0,                        // top
                      WIN_WIDTH,                // width of windows
                      WIN_HEIGHT,               // height of window
                      0,                        // border width
                      visualInfo.depth,         // depth fo window
                      InputOutput,              // window class
                      visualInfo.visual,        // visual info
                      CWColormap | CWEventMask, // style mask
                      &xattr                    // pointer to attributes
    );

    /* register for window close event */
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);

    XStoreName(dpy, w, "Rohit Nimkar: Vulkan");
    XMapWindow(dpy, w);

    int screenWidth  = XWidthOfScreen(XScreenOfDisplay(dpy, visualInfo.screen));
    int screenHeight = XHeightOfScreen(ScreenOfDisplay(dpy, visualInfo.screen));
    XMoveWindow(dpy, w, (screenWidth - WIN_WIDTH) / 2, (screenHeight - WIN_HEIGHT) / 2);
    XFlush(dpy);

    initialize();

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
                    if (event.xclient.data.l[0] == wm_delete_window)
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
                        case XK_a:
                        {
                            if (event.xkey.state & ShiftMask)
                            {
                                /* handle A */
                            }
                            else
                            {
                            }
                            break;
                        }
                        case XK_f:
                        {
                            toggleFullscreen(dpy, w);
                            break;
                        }

                        case XK_r:
                        {
                            break;
                        }
                        case XK_Escape:
                        {
                            gbAbortFlag = true;
                            break;
                        }
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
    }

    uninitialize();
    XFreeColormap(dpy, colormap);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);

    return 0;
}

static int initialize()
{
    XWindowAttributes xattr = {};
    XGetWindowAttributes(dpy, w, &xattr);

    initVulkan();

    // Set the clipping plane equation
    resize(xattr.width, xattr.height);
    // toggleFullscreen(dpy, w);
    return (0);
}

void uninitialize()
{
    cleanUp();
}

static void display()
{
    drawFrame();
}

static void update()
{
}

static void resize(int width, int height)
{
}

static void toggleFullscreen(Display *display, Window window)
{
    XEvent evt;

    Atom wm_state   = XInternAtom(display, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    evt.xclient.type         = ClientMessage;
    evt.xclient.serial       = 0;
    evt.xclient.send_event   = True;
    evt.xclient.message_type = wm_state;
    evt.xclient.format       = 32;
    evt.xclient.window       = window;
    evt.xclient.data.l[0]    = gbFullscreen ? 0 : 1; // _NET_WM_STATE_TOGGLE
    evt.xclient.data.l[1]    = fullscreen;
    evt.xclient.data.l[2]    = 0;

    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &evt);
    gbFullscreen = !gbFullscreen;
}
