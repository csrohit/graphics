#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

Display *display;
Window window;
void drawTriangle() {
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0);  // Red
    glVertex2f(0.0, 1.0);

    glColor3f(0.0, 1.0, 0.0);  // Green
    glVertex2f(-1.0, -1.0);

    glColor3f(0.0, 0.0, 1.0);  // Blue
    glVertex2f(1.0, -1.0);
    glEnd();
}


void DrawBWTriangle(void)
{
    glBegin(GL_TRIANGLES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 0.0f);

    glEnd();
}

void DrawBWRectangle(void)
{
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 0.0f);

    glEnd();
}

void initialize()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 4.0f/3.0f, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void displa()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();             // take identity matrix for beginning

    glTranslatef(-1.5f, 0.0f, -6.0f);
    DrawBWTriangle();

    glLoadIdentity(); // take identity matrix for beginning
    glTranslatef(1.5f, 0.0f, -6.0f);
    DrawBWRectangle();
    glXSwapBuffers(display, window);
}

void mainLoop()
{
    XEvent event;
    while (1) {
        if(XPending(display))
        {
            XNextEvent(display, &event);
            switch (event.type) {
                case Expose:
                    break;
                case KeyPress:
                    XCloseDisplay(display);
                    return ;
            }
        }
        displa();
    }
}

int main() {
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open display.\n");
        return 1;
    }

    int screen = DefaultScreen(display);

    Window root = RootWindow(display, screen);

    // Set up a visual with double buffering
    static int attributeList[] = {GLX_RGBA, GLX_DOUBLEBUFFER, None};
    XVisualInfo *visual = glXChooseVisual(display, screen, attributeList);
    if (!visual) {
        fprintf(stderr, "No appropriate visual found.\n");
        return 1;
    }

    Colormap colormap = XCreateColormap(display, root, visual->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = colormap;
    swa.event_mask = ExposureMask | KeyPressMask;
    window = XCreateWindow(display, root, 0, 0, 800, 600, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &swa);

    XMapWindow(display, window);
    XStoreName(display, window, "OpenGL Triangle");

    GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
    glXMakeCurrent(display, window, context);
    initialize();
    mainLoop();
    return 0;
}



