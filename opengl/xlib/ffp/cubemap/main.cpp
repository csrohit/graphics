#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "stb_image.h"

static void quadloop(GLfloat r, GLfloat R, GLint nsides, GLfloat sideDelta, GLfloat cosTheta, GLfloat sinTheta, GLfloat cosTheta1, GLfloat sinTheta1);
static void doughnut(GLfloat r, GLfloat R, GLint nsides, GLint rings);

/* function declaration */
static void initialize();
static void uninitialize();
static void display();
static void update();
static void resize(GLsizei width, GLsizei height);
static void toggleFullscreen(Display *display, Window window);
void        DrawGround(void);

/* Windowing related variables */
Display   *dpy          = nullptr; // connection to server
Window     w            = 0UL;     // handle of current window
bool       gbAbortFlag  = false;   // Global abort flag
GLint      result       = 0;       // variable to get value returned by APIS
GLXContext glCtxt       = nullptr; // handle to OpenGL context
XRectangle rect         = {0};     // window dimentions rectangle
bool       gbFullscreen = false;   // should display in fullscreen mode
bool       shouldDraw   = false;   // should scene be rendered

/*--- Program specific variables ---*/
GLfloat     colorWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat     colorBlack[4] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat     zPos          = 0.1f;
GLfloat     xPos          = 0.1f;
GLUquadric *pQuadric;

/* Light properties */
GLfloat lightPosition[4]       = {3.0f, 0.0f, 2.0f, 1.0f};
GLfloat lightPositionMirror[4] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightAmbient[4]        = {0.25, 0.25, 0.25, 1.0f};
GLfloat lightDiffuse[4]        = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[4]       = {1.0f, 1.0f, 1.0f, 1.0f};

GLfloat materialBlue[4]   = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat materialGreen[4]  = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat materialYellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
GLfloat materialCyan[4]   = {0.0f, 1.0f, 1.0f, 1.0f};

/* Material Diffuse */
GLfloat materialDiffuse[4] = {1.0f, 0.0f, 0.0f, 1.0f};
/*-----*/
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

    // clang-format off
    GLint glxAttriutes[] = {
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
    // clang-format on

    XVisualInfo *visual = glXChooseVisual(dpy, screen, glxAttriutes);
    if (visual == nullptr)
    {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    XSetWindowAttributes xattr;
    xattr.event_mask        = ExposureMask | KeyPressMask;
    xattr.border_pixel      = BlackPixel(dpy, screen);
    xattr.background_pixel  = WhitePixel(dpy, screen);
    xattr.override_redirect = true;
    xattr.colormap          = XCreateColormap(dpy, root, visual->visual, AllocNone);
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask;

    w = XCreateWindow(dpy, root, 0, 0, 800, 600, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &xattr);
    /* register for window close event */
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);

    XStoreName(dpy, w, "Rohit Nimkar: Doughnut");
    XMapWindow(dpy, w);

    glCtxt = glXCreateContext(dpy, visual, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, glCtxt);
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
                                zPos += 0.1;
                            }
                            else
                            {
                                zPos -= 0.1;
                            }
                            break;
                        }
                        case XK_f:
                        {
                            toggleFullscreen(dpy, w);
                            gbFullscreen = !gbFullscreen;
                            break;
                        }
                        case XK_l:
                        {
                            static bool bLight = false;
                            if (false == bLight)
                            {
                                glEnable(GL_LIGHTING);
                            }
                            else
                            {
                                glDisable(GL_LIGHTING);
                            }
                            bLight = !bLight;
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

        glXSwapBuffers(dpy, w);
    }

    uninitialize();
    glXMakeCurrent(dpy, None, nullptr);
    glXDestroyContext(dpy, glCtxt);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);

    return 0;
}

GLuint torus;

void makeImages(void)
{
    const char *names[] = {"res/right.jpg", "res/left.jpg", "res/top.jpg", "res/bottom.jpg", "res/front.jpg", "res/back.jpg"};
    int         width, height, nrChannels;

    for (uint32_t idx = 0U; idx < 6U; ++idx)
    {
        unsigned char *data = stbi_load(names[idx], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + idx, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
    }
}
void drawCube()
{
    glBegin(GL_QUADS);

    /* Front face */
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);   // top-right
    glVertex3f(-1.0f, 1.0f, 1.0f);  // top-left
    glVertex3f(-1.0f, -1.0f, 1.0f); // bottom-left
    glVertex3f(1.0f, -1.0f, 1.0f);  // bottom right

    /* Right face */
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);  // top-right
    glVertex3f(1.0f, 1.0f, 1.0f);   // top-left
    glVertex3f(1.0f, -1.0f, 1.0f);  // bottom-left
    glVertex3f(1.0f, -1.0f, -1.0f); // bottom-right

    /* Back face */
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);  // top-right
    glVertex3f(1.0f, 1.0f, -1.0f);   // top-left
    glVertex3f(1.0f, -1.0f, -1.0f);  // bottom left
    glVertex3f(-1.0f, -1.0f, -1.0f); // bottom-right

    /* Left face */
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);   // top-right
    glVertex3f(-1.0f, 1.0f, -1.0f);  // top-left
    glVertex3f(-1.0f, -1.0f, -1.0f); // bottom-left
    glVertex3f(-1.0f, -1.0f, 1.0f);  // bottom-right

    /* Top face */
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);  // top-right
    glVertex3f(1.0f, 1.0f, 1.0f);   // top-left
    glVertex3f(1.0f, 1.0f, -1.0f);  // bottom-left
    glVertex3f(-1.0f, 1.0f, -1.0f); // bottom-right

    /* Bottom face */
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);   // top-right
    glVertex3f(-1.0f, -1.0f, 1.0f);  // top-left
    glVertex3f(-1.0f, -1.0f, -1.0f); // bottom-left
    glVertex3f(1.0f, -1.0f, -1.0f);  // bottom-right

    glEnd();
}


static void initialize()
{
    XWindowAttributes xattr;
    XGetWindowAttributes(dpy, w, &xattr);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClearDepth(1.0f);      // this bit will be set in depth buffer after calling glClear()
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LEQUAL);  // Which function to use for testing
    glEnable(GL_CULL_FACE);
    // glFrontFace(GL_CW);
    // glEnable(GL_CULL_FACE);

    /* per light initialization */
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    makeImages();
    // glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT);
    // glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT);
    // glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_CUBE_MAP_EXT);
    // glEnable(GL_LIGHTING);
    // glEnable(GL_LIGHT0);
    // glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
    
    pQuadric = gluNewQuadric();

    torus = glGenLists(1);
    glNewList(torus, GL_COMPILE);
    glTranslatef(-4.0f, 0.0f, 0.0f);
    gluSphere(pQuadric, 1.0f, 100, 100);
    glTranslatef(4.0f, 0.0f, 0.0f);
    
    drawCube();

    glTranslatef(4.0f, 0.0f, 0.0f);
    doughnut(0.3f, 0.8f, 50, 50);
    glEndList();

    // Set the clipping plane equation
    resize(xattr.width, xattr.height);
    toggleFullscreen(dpy, w);
}

void uninitialize()
{
    glDeleteLists(torus, 1);
    gluDeleteQuadric(pQuadric);
}

float angle = 0.0f;

void   drawScene();

static void display()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    gluLookAt(xPos, 2.0f, zPos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Load and enable the appropriate texture for each face of the cube
    // Assuming you have loaded and bound textures for each face (front, back, left, right, top, bottom)

    glCallList(torus);
}


static void update()
{
    angle += 0.01;
    if (angle >= 360.0f)
    {
        angle -= 360.0f;
    }
    xPos = 8.0f * cosf(angle);
    zPos = 8.0f * sinf(angle);
    // xPos = 8.0f;
    // zPos = 38.0f;
}

static void resize(GLsizei width, GLsizei height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glViewport(0, 0, width, height);
}

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
    evt.xclient.data.l[0]    = 2; // _NET_WM_STATE_TOGGLE
    evt.xclient.data.l[1]    = fullscreen;
    evt.xclient.data.l[2]    = 0;

    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &evt);
}

void DrawGround(void)
{
    GLfloat fExtent = 5.0f;
    GLfloat fStep   = 0.5f;
    GLfloat y       = 0.0f;
    GLint   iBounce = 0;
    GLfloat iStrip, iRun, fColor;

    glShadeModel(GL_FLAT);
    for (iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
        {
            if ((iBounce % 2) == 0)
                fColor = 1.0f;
            else
                fColor = 0.0f;

            glColor4f(fColor, fColor, fColor, 0.5f);
            glVertex3f(iStrip, y, iRun);
            glVertex3f(iStrip + fStep, y, iRun);

            iBounce++;
        }
        glEnd();
    }
    glShadeModel(GL_SMOOTH);
}
