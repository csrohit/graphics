#define __USE_MATH_DEFINES
#include <math.h>

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <cstdio>
#include <cstdlib>

#include "fourier.h"

const int gwidth  = 2880;
const int gheight = 1740;

Display   *display;             // pointer to display
Window     window;              // handle of main window
GLXContext glContext;           // handle to opengl context
XRectangle rect        = {0};   // window dimentions rectangle
bool       gbAbortFlag = false; // globake abort flag

void resize(GLsizei width, GLsizei height)
{
    if (height <= 0)
        height = 1;

    glMatrixMode(GL_PROJECTION); // for matrix calculation while resizing use GL_PROJECTION
    glLoadIdentity();            // take identity matrix for beginning

    GLdouble gldHeight = (GLdouble)(tan(M_PI / 8.0f) * 0.1);
    GLdouble gldWidth  = gldHeight * ((GLdouble)width / (GLdouble)height);

    /* after setting projection matrix, ensure that VIEW matrix is also set */
    glFrustum(-gldWidth, gldWidth, -gldHeight, gldHeight, 0.1, 100.0);
    gluLookAt(0.0, 0.0, 20.0f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glViewport(0, 0, width, height); // view complete window
}

void createWindow()
{
    display = XOpenDisplay(nullptr);

    if (display == nullptr)
    {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(display);

    // Define the attributes for the visual context
    static int visualAttributes[] = {
        GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_SAMPLE_BUFFERS, 0, GLX_SAMPLES, 0,
        None};

    XVisualInfo *visual = glXChooseVisual(display, screen, visualAttributes);
    if (visual == nullptr)
    {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    Colormap             colormap = XCreateColormap(display, RootWindow(display, screen), visual->visual, AllocNone);
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap   = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    window = XCreateWindow(display, RootWindow(display, screen), 0, 0, gwidth, gheight, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

    XMapWindow(display, window);
    XStoreName(display, window, "OpenGL Fourier");

    glContext = glXCreateContext(display, visual, nullptr, GL_TRUE);
    glXMakeCurrent(display, window, glContext);
    resize(gwidth, gheight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    drawSqaure();
    glXSwapBuffers(display, window);
}

int main(int argc, char *argv[])
{
    createWindow();
    initializeFourier();

    while (false == gbAbortFlag)
    {
        XEvent event;
        if (XPending(display))
        {

            XNextEvent(display, &event);
            switch (event.type)
            {
                case Expose:
                {
                    renderScene();
                    break;
                }

                case KeyPress:
                {

                    KeySym sym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);

                    switch (sym)
                    {
                        case XK_a:
                        {
                                    extern int gn;
                            if (event.xkey.state & ShiftMask)
                            {
                                /* handle A */
                                        gn+=2;
                            }
                            else
                            {
                                        gn-=2;
                            }
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
            }
        }
        renderScene();
    }
    freeFourier();
    glXMakeCurrent(display, None, nullptr);
    glXDestroyContext(display, glContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
