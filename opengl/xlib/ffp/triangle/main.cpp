#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdio>
#include <cstdlib>
#define _USE_MATH_DEFINES
#include <math.h>


static void quadloop(GLfloat r, GLfloat R, GLint nsides, GLfloat sideDelta, GLfloat cosTheta, GLfloat sinTheta, GLfloat cosTheta1, GLfloat sinTheta1)
{
    GLfloat dist, phi;
    int     j;

    glBegin(GL_QUAD_STRIP);

    dist = R + r;
    glNormal3f(cosTheta1, -sinTheta1, 0);
    glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, 0);
    glNormal3f(cosTheta, -sinTheta, 0);
    glVertex3f(cosTheta * dist, -sinTheta * dist, 0);

    phi = sideDelta;
    for (j = nsides - 2; j >= 0; j--)
    {
        GLfloat cosPhi, sinPhi;

        cosPhi = cos(phi);
        sinPhi = sin(phi);
        dist   = R + r * cosPhi;

        glNormal3f(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi);
        glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, r * sinPhi);
        glNormal3f(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi);
        glVertex3f(cosTheta * dist, -sinTheta * dist, r * sinPhi);
        phi += sideDelta;
    }

    /* Repeat first two vertices to seam up each quad strip loop so no cracks. */
    dist = R + r;
    glNormal3f(cosTheta1, -sinTheta1, 0);
    glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, 0);
    glNormal3f(cosTheta, -sinTheta, 0);
    glVertex3f(cosTheta * dist, -sinTheta * dist, 0);

    glEnd();
}

static void doughnut(GLfloat r, GLfloat R, GLint nsides, GLint rings)
{
    const GLfloat ringDelta = 2.0 * M_PI / rings;
    const GLfloat sideDelta = 2.0 * M_PI / nsides;

    GLfloat theta, theta1;
    GLfloat cosTheta, sinTheta;
    GLfloat cosTheta1, sinTheta1;
    int     i;

    theta    = 0.0;
    cosTheta = 1.0;
    sinTheta = 0.0;
    for (i = rings - 2; i >= 0; i--)
    {
        theta1    = theta + ringDelta;
        cosTheta1 = cos(theta1);
        sinTheta1 = sin(theta1);

        quadloop(r, R, nsides, sideDelta, cosTheta, sinTheta, cosTheta1, sinTheta1);

        theta    = theta1;
        cosTheta = cosTheta1;
        sinTheta = sinTheta1;
    }

    cosTheta1 = 1.0;
    sinTheta1 = 0.0;
    quadloop(r, R, nsides, sideDelta, cosTheta, sinTheta, cosTheta1, sinTheta1);
}

Display *display;
Window window;
GLXContext glContext;

void createWindow() {
    display = XOpenDisplay(nullptr);

    if (display == nullptr) {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(display);

    // Define the attributes for the visual context
    static int visualAttributes[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_SAMPLE_BUFFERS, 0,
        GLX_SAMPLES, 0,
        None
    };

    XVisualInfo *visual = glXChooseVisual(display, screen, visualAttributes);
    if (visual == nullptr) {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    Colormap colormap = XCreateColormap(display, RootWindow(display, screen), visual->visual, AllocNone);
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask;

    window = XCreateWindow(display, RootWindow(display, screen), 0, 0, 800, 600, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

    XMapWindow(display, window);
    XStoreName(display, window, "OpenGL Triangle");

    glContext = glXCreateContext(display, visual, nullptr, GL_TRUE);
    glXMakeCurrent(display, window, glContext);
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)4 / (GLfloat)3, 0.1f, 100.0f);
    gluLookAt(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);
    glRotatef(0.5f, 0.0f, 0.0f, 1.0f);
    
    doughnut(0.2f, 0.9f, 12, 12);

    glXSwapBuffers(display, window);
}

int main(int argc, char *argv[]) {
    createWindow();

    while (1) {
        XEvent event;
        if(XPending(display))
        {

        XNextEvent(display, &event);

        if (event.type == Expose) {
            renderScene();
        }

        if (event.type == KeyPress) {
            break;
        }
        }
            renderScene();
    }

    glXMakeCurrent(display, None, nullptr);
    glXDestroyContext(display, glContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}

