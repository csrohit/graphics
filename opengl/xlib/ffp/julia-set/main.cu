
#include <cstdint>
#include <iostream>
#define USE_MATH_DEFINES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#define WIN_WIDTH  800
#define WIN_HEIGHT 600

void        HSBtoRGB(double hue, double saturation, double brightness, GLubyte *out);
Display    *dpy              = nullptr; // connection to x-server
Window      w                = 0UL;     // handle to current window
GLXContext  glCtxt           = nullptr; // pointer to opengl context
bool        gbAbortFlag      = false;
bool        gbFullscreen     = false;
static Atom wm_delete_window = 0; // atomic variable to detect close button click

XRectangle rect                 = {0}; // window dimentions rectangle
GLuint     uTextureCheckerBoard = 0U;

#define WIDTH    2400  // Image width
#define HEIGHT   1800  // Image height
#define MAX_ITER 1000  // Maximum number of iterations for each pixel
#define CXMIN    -2.0f // Minimum real value for the subset
#define CXMAX    2.0f  // Maximum real value for the subset
#define CYMIN    -1.5f // Minimum imaginary value for the subset
#define CYMAX    2.5f  // Maximum imaginary value for the subset
int     output[WIDTH][HEIGHT]        = {};
GLubyte checkImage[WIDTH][HEIGHT][4] = {0}; // OpenGL is column major

float cx = -0.7269;// -0.8f; //0.0f;
    float cy = 0.1889; //0.156f; //0.0f;

int  *dev_output;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB;

void writePPM(const char *filename)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(file, "P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for (uint32_t idx = 0U; idx < WIDTH; idx++)
    {
        for (uint32_t idy = 0U; idy < HEIGHT; idy++)
        {
            int   val = output[idx][idy];
            if(val < MAX_ITER)
            { 
            float t   = ((float)val * 360.0f) / MAX_ITER;
            HSBtoRGB(t, 1.0, 1.0, checkImage[idx][idy]);
            }
            else {
                checkImage[idx][idy][0] = 0;
                checkImage[idx][idy][1] = 0;
                checkImage[idx][idy][2] = 0;
            }
            fprintf(file, "%d %d %d\n", checkImage[idx][idy][0], checkImage[idx][idy][1], checkImage[idx][idy][2]);
        }
    }

    fclose(file);
}
void HSBtoRGB(double hue, double saturation, double brightness, GLubyte *out)
{
    hue        = fmod(hue, 360.0);                                                   // Ensure hue is within [0, 360) degrees
    saturation = (saturation > 1.0) ? 1.0 : ((saturation < 0.0) ? 0.0 : saturation); // Limit saturation within [0, 1]
    brightness = (brightness > 1.0) ? 1.0 : ((brightness < 0.0) ? 0.0 : brightness); // Limit brightness within [0, 1]

    double c = saturation * brightness;
    double x = c * (1 - fabs(fmod(hue / 60.0, 2) - 1));
    double m = brightness - c;

    double r1, g1, b1;

    if (hue >= 0 && hue < 60)
    {
        r1 = c;
        g1 = x;
        b1 = 0;
    }
    else if (hue >= 60 && hue < 120)
    {
        r1 = x;
        g1 = c;
        b1 = 0;
    }
    else if (hue >= 120 && hue < 180)
    {
        r1 = 0;
        g1 = c;
        b1 = x;
    }
    else if (hue >= 180 && hue < 240)
    {
        r1 = 0;
        g1 = x;
        b1 = c;
    }
    else if (hue >= 240 && hue < 300)
    {
        r1 = x;
        g1 = 0;
        b1 = c;
    }
    else
    {
        r1 = c;
        g1 = 0;
        b1 = x;
    }

    out[0] = (uint8_t)((r1 + m) * 255);
    out[1] = (uint8_t)((g1 + m) * 255);
    out[2] = (uint8_t)((b1 + m) * 255);
}
__global__ void mandelbrot(float cx, float cy, int *output)
{
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < HEIGHT && col < WIDTH)
    {
        float    zx = CXMIN + (CXMAX - CXMIN) * col / WIDTH;
        float    zy = CYMIN + (CYMAX - CYMIN) * row / WIDTH;
        uint32_t n  = 0;
        for (n = 0; n < MAX_ITER; n++)
        {
            if ((zx * zx + zy * zy) >= 4.0f)
            {
                break;
            }
            float temp = zx * zx - zy * zy + cx;
            zy         = 2 * zx * zy + cy;
            zx         = temp;
            n++;
        }
        output[row * WIDTH + col] = n;
    }
}
static void toggleFullscreen(Display *display, Window window)
{
    XEvent xev;

    Atom wm_state   = XInternAtom(display, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    xev.xclient.type         = ClientMessage;
    xev.xclient.serial       = 0;
    xev.xclient.send_event   = True;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.window       = window;
    xev.xclient.data.l[0]    = 2; // _NET_WM_STATE_TOGGLE
    xev.xclient.data.l[1]    = fullscreen;
    xev.xclient.data.l[2]    = 0;

    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}
void makeCheckImage(void)
{
    // Define grid and block dimensions for kernel execution
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((WIDTH + threadsPerBlock.x - 1) / threadsPerBlock.x, (HEIGHT + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Launch kernel
    mandelbrot<<<numBlocks, threadsPerBlock>>>(cx, cy, dev_output);

    // Copy result from device to host
    cudaMemcpy(output, dev_output, WIDTH * HEIGHT * sizeof(int), cudaMemcpyDeviceToHost);

    // Write computed Mandelbrot set to a PPM file
    for (uint32_t idx = 0U; idx < WIDTH; idx++)
    {
        for (uint32_t idy = 0U; idy < HEIGHT; idy++)
        {
            
            int   val = output[idx][idy];
            if(val < MAX_ITER)
            {
                float t   = ((float)val / MAX_ITER) * 460.0f;
                HSBtoRGB(t, 1.0, 1.0, checkImage[idx][idy]);
            }
            else {
                checkImage[idx][idy][0] = 0;
                checkImage[idx][idy][1] = 0;
                checkImage[idx][idy][2] = 0;
            }

        }
    }
    // Free device and host memory
}
void loadGLTexture1(void)
{
    /* Local variable declarations */
    makeCheckImage();

    /* Bind Generated texture */
    glBindTexture(GL_TEXTURE_2D, uTextureCheckerBoard);

    // alignment and unpacking
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /* Set Texture parameters
        Nearest - Better for processing and comparatively poor on quality
        Linear - Poor in processing and better on quality
    */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    /* build mipmap images */
    // gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, (void *)bmp.bmBits);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)checkImage);

    /* Unbind texture */
    glBindTexture(GL_TEXTURE_2D, 0);
}

void createWindow()
{
    dpy = XOpenDisplay(nullptr);

    if (dpy == nullptr)
    {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(dpy);

    // Define the attributes for the visual context
    static int visualAttributes[] = {
        GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_SAMPLE_BUFFERS, 0, GLX_SAMPLES, 0,
        None};

    XVisualInfo *visual = glXChooseVisual(dpy, screen, visualAttributes);
    if (visual == nullptr)
    {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    Colormap             colormap = XCreateColormap(dpy, RootWindow(dpy, screen), visual->visual, AllocNone);
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap   = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    w = XCreateWindow(dpy, RootWindow(dpy, screen), 0, 0, WIN_WIDTH, WIN_HEIGHT, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

    XMapWindow(dpy, w);
    XStoreName(dpy, w, "OpenGL Triangle");
    toggleFullscreen(dpy, w);

    glCtxt = glXCreateContext(dpy, visual, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, glCtxt);

    // Allocate memory for output
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    cudaMalloc((void **)&dev_output, WIDTH * HEIGHT * sizeof(int));
    // Allocate memory on the GPU
    /* Create OpenGL Texture Object */
    glGenTextures(1, &uTextureCheckerBoard);
    // loading images to create texture

    loadGLTexture1();
    /* Enable texture */
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderScene()
{
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window with color whose bit is set
    glMatrixMode(GL_MODELVIEW);                         // for matrix calculation while displaying use GL_MODELVIEW beacuse need to display something
    glLoadIdentity();                                   // take identity matrix for beginning

    /* Do initial tranlation for better visibility */
    // glTranslatef(0.0f, 0.0f, -4.0f);

    glBindTexture(GL_TEXTURE_2D, uTextureCheckerBoard);
    glBegin(GL_QUADS);

    /* Top-Right */
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);

    /* Top-Left */
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 0.0f);

    /* Bottom-Left */
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 0.0f);

    /* Bottom-Right */
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 0.0f);

    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}
void resize(int width, int height)
{
    if (height <= 0)
        height = 1;

    glMatrixMode(GL_PROJECTION); // for matrix calculation while resizing use GL_PROJECTION
    glLoadIdentity();            // take identity matrix for beginning

    GLdouble gldHeight = (GLdouble)(tan(M_PI / 8.0f) * 0.1);
    GLdouble gldWidth  = gldHeight * ((GLdouble)width / (GLdouble)height);

    /* after setting projection matrix, ensure that VIEW matrix is also set */
    // glFrustum(-gldWidth, gldWidth, -gldHeight, gldHeight, 0.1, 100.0);
    // gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glViewport(0, 0, width, height); // view complete window
}

void update()
{
}

int main(int argc, char *argv[])
{
    createWindow();
    bool shouldDraw = false;
    shouldDraw      = false;
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
                            writePPM("key.ppm");
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
                            gbFullscreen = !gbFullscreen;
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
        renderScene();

        glXSwapBuffers(dpy, w);
    }
    cudaFree(dev_output);
    glDeleteTextures(1, &uTextureCheckerBoard);
    glXMakeCurrent(dpy, None, nullptr);
    glXDestroyContext(dpy, glCtxt);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);

    return 0;
}
