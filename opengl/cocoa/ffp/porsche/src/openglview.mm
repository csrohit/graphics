/**
 * @file        openglview.mm
 * @description opengl view event handler
 * @author      Rohit Nimkar
 * @version     1.0
 * @date        2023-12-10
 * @copyright   Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */

#import <Cocoa/Cocoa.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#import "openglview.h"
#include "vmath.h"
#include "car.h"

GLfloat globalAmbient[]        = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat ground[]               = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat cosmisAmbientDefault[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat black[4]               = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat *pLightColor0          = black;
GLfloat *pLightColor1           = black;

GLfloat lightAmbient0[]        = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuse0[]        = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightSpecular0[]       = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition0[]       = {2.0f, 0.0f, 0.0f, 1.0f};

GLfloat lightAmbient1[]        = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuse1[]        = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightSpecular1[]       = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPosition1[]       = {-2.0f, 0.0f, 0.0f, 1.0f};

GLfloat materialAmbient[]      = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialDiffuse[]      = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[]     = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess      = 128.0f;

GLfloat temp[4];

/* CVDisplayCallback */
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                               CVOptionFlags, CVOptionFlags*, void*);
void         printValues();

extern FILE *gpFile;

@implementation MyOpenGLView {
    @private
        GLfloat rotationAngle;
        CVDisplayLinkRef displayLink;
        GLUquadric* pQuadric;

}

    - (instancetype)initWithFrame:(NSRect)frameRect {

        /*       
            NSOpenGLPFAWindow,
            NSOpenGLProfileVersionLegacy
            ---- above profile or below profile both work ----
            NSOpenGLPFAWindow
            ---- study and analyse which one is used for what purpose ----
        */
        GLuint attributes[] =
        {
            NSOpenGLPFAOpenGLProfile,
            NSOpenGLProfileVersionLegacy,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAMinimumPolicy,
            0
        };

        NSOpenGLPixelFormat* pixelformat =
            [ [ NSOpenGLPixelFormat alloc ] initWithAttributes:
                (NSOpenGLPixelFormatAttribute*) attributes ];

        if ( pixelformat == nil )
        {
            /* 
                failed to find requested pixel formats
                We can try requesting the different pixel formats instead
                If we are still not able to create a proper rendering context
                then gracefully exit the application
            */
            fprintf(gpFile, "No valid OpenGL pixel format" );
            [NSApp stop:self];
        }
        
        /* Initialize View with obtained pixel format */
        return self = [ super initWithFrame: frameRect
                                pixelFormat: [ pixelformat autorelease ] ];
    }

    -(CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
    {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        if(rotationAngle > 360.0f)
            rotationAngle = rotationAngle - 360.0f;
        rotationAngle += 0.5f;
        [self display];
        [pool release];

        return (kCVReturnSuccess);
    }

    - (void)dealloc {
        if (nullptr != pQuadric)
        {
            gluDeleteQuadric(pQuadric);
            pQuadric = nullptr;
        }

        freeCar();
        CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);
        [super dealloc];
    }

    - (void)drawRect:(NSRect)rect {
        [super drawRect:rect];
        [self display];
    }

    -(void)prepareOpenGL {
        /*---- Load models --*/
        initializeCar();

        [super prepareOpenGL];
        
        /* setup display link */
        CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
        CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
        CVDisplayLinkStart(displayLink);

        NSOpenGLContext *openGLContext = self.openGLContext;
        [openGLContext makeCurrentContext];
            
        /* Scene initialization */
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_CULL_FACE);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        printValues();
        //set up light 0 properties
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse0);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);

        //set up light 1 properties
        glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse1);
        glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular1);
        glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);

        //set up material properties
        glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);
    /* Initialize quadric */
    pQuadric = gluNewQuadric();
        [self reshape];
    }

    -(BOOL)acceptsFirstResponder
    {
        //code
        [[self window] makeFirstResponder:self];
        return (YES);
    }

    -(void)keyDown:(NSEvent *)event {
        int key = [[event characters] characterAtIndex:0];
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        switch(key)
        {
            case 27:
                [[self window] close];
                break;
            
            case 'F':
            case 'f':
                [[self window] toggleFullScreen:self];
                break;

            case 'r':
            case 'R':
                static bool bIsLight0Enabled = false;
                if(false == bIsLight0Enabled)
                {
                    [self toggleLighting];
                    glEnable(GL_LIGHT0);
                    pLightColor0 = lightDiffuse0;
                    [self toggleLighting];
                }
                else
                {    
                    [self toggleLighting];
                    glDisable(GL_LIGHT0);
                    pLightColor0 = black;
                    [self toggleLighting];
                }
                bIsLight0Enabled = !bIsLight0Enabled;
                break;

            case 'g':
            case 'G':
                static bool bIsLight1Enabled = false;
                if(false == bIsLight1Enabled)
                {
                    [self toggleLighting];
                    glEnable(GL_LIGHT1);
                    pLightColor1 = lightDiffuse1;
                    [self toggleLighting];
                }
                else
                {
                    [self toggleLighting];
                    glDisable(GL_LIGHT1);
                    pLightColor1 = black;
                    [self toggleLighting];
                }
                bIsLight1Enabled = !bIsLight1Enabled;
                break;
            case 'L':
            case 'l':
                [self toggleLighting];
                break; 
            case 'm':
            case 'M':
                static bool bIsCosmicEnabled = false;
                if(false == bIsCosmicEnabled)
                {
                    [self toggleLighting];
                    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
                    [self toggleLighting];
                }
                else
                {
                    [self toggleLighting];
                    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, cosmisAmbientDefault);
                    [self toggleLighting];
                }
                bIsCosmicEnabled = !bIsCosmicEnabled;
        }
        CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
        CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    }
    - (void)toggleLighting
    {
        static bool bIsLightingEnabled = false;
        if(false == bIsLightingEnabled)
        {
            glEnable(GL_LIGHTING);
        }
        else {
            glDisable(GL_LIGHTING);
        }
        bIsLightingEnabled = !bIsLightingEnabled;
    }

    - (void) display
    {
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        
        /*--- draw here ---*/

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glColor3f(0.5f, 0.5f, 0.5f);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ground);
        glTranslatef(0.0f, -2.0f, -3.0f);
        glScalef(5.0f, 1.0f, 5.0f);
        glBegin(GL_QUADS);
        glVertex3f(-1.0f, 0.0f, -1.0f);
        glVertex3f(1.0f, 0.0f, -1.0f);
        glVertex3f(1.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, 0.0f, 1.0f);
        glEnd();


        glColor3f(1.0f, 1.0f, 1.0f);
    
        glLoadIdentity();
        glPushMatrix();
        glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);
        displayCar();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(lightPosition0[0], lightPosition0[1], lightPosition0[2]);
        glMaterialfv(GL_FRONT, GL_EMISSION, pLightColor0);
        gluSphere(pQuadric, 0.1f, 40, 40); // it will create all normals for you
        glPopMatrix();

        glPushMatrix();
        glTranslatef(lightPosition1[0], lightPosition1[1], lightPosition1[2]);
        glMaterialfv(GL_FRONT, GL_EMISSION, pLightColor1);
        gluSphere(pQuadric, 0.1f, 40, 40); // it will create all normals for you
        glMaterialfv(GL_FRONT, GL_EMISSION, black);
        glPopMatrix();

        /*--- ** ---*/
        glFlush();
        CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
        CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    }

- (void) reshape {
    [super reshape];
    NSRect rect = [self bounds];

    CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);

    if(rect.size.height < 0)
        rect.size.height = 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)rect.size.width / (GLfloat)rect.size.height, 0.1f, 100.0f);
    
    gluLookAt(0.0, 2.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    /* @note: while writing this code the following issue is observed hence multiplying by 2
       https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value 
    */
    glViewport(0, 0, (GLsizei)rect.size.width*2, (GLsizei)rect.size.height*2);
    
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}
@end

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime,
                               CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *displayLinkContext)
{
    CVReturn result = [(MyOpenGLView*)displayLinkContext getFrameForTime:outputTime];
    return (result);
}


void printValues()
{
    /* Ambient RGBA Lighting of entire scene */
    glGetFloatv(GL_LIGHT_MODEL_AMBIENT, cosmisAmbientDefault);
    fprintf(gpFile, "[Initialize] [Cosmic Background Radiation] Ambient => (%.2f, %.2f, %.2f, %.2f)\n", cosmisAmbientDefault[0], cosmisAmbientDefault[1], cosmisAmbientDefault[2], cosmisAmbientDefault[3]);

    /* How specular reflectin angles are computed */
    glGetFloatv(GL_LIGHT_MODEL_LOCAL_VIEWER, temp);
    fprintf(gpFile, "[Initialize] [Cosmic Background Radiation] Local Viewer => (%.2f)\n", temp[0]);

    /* isOneside of Tow sided lighting? */
    glGetFloatv(GL_LIGHT_MODEL_TWO_SIDE, temp);
    fprintf(gpFile, "[Initialize] [Cosmic Background Radiation] Two Side => (%.2f)\n", temp[0]);

    glGetMaterialfv(GL_FRONT, GL_AMBIENT, temp);
    fprintf(gpFile, "[Initialize] [Material Front] Ambient Reflectance => (%.2f, %.2f, %.2f, %.2f)\n", temp[0], temp[1], temp[2], temp[3]);
    glGetMaterialfv(GL_FRONT, GL_DIFFUSE, temp);
    fprintf(gpFile, "[Initialize] [Material Front] Diffuse Reflectance => (%.2f, %.2f, %.2f, %.2f)\n", temp[0], temp[1], temp[2], temp[3]);
    glGetMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, temp);
    fprintf(gpFile, "[Initialize] [Material Front] Ambient & Diffuse Reflectance => (%.2f, %.2f, %.2f, %.2f)\n", temp[0], temp[1], temp[2], temp[3]);
    glGetMaterialfv(GL_FRONT, GL_SPECULAR, temp);
    fprintf(gpFile, "[Initialize] [Material Front] Specular Reflectance => (%.2f, %.2f, %.2f, %.2f)\n", temp[0], temp[1], temp[2], temp[3]);

    /* Appears like emitting color, does notactually emit color */
    glGetMaterialfv(GL_FRONT, GL_EMISSION, temp);
    fprintf(gpFile, "[Initialize] [Material Front] Emission => (%.2f, %.2f, %.2f, %.2f)\n", temp[0], temp[1], temp[2], temp[3]);

    /* concentration of highlight of Specular component */
    glGetMaterialfv(GL_FRONT, GL_SHININESS, temp);
    fprintf(gpFile, "[Initialize] [Material Front] Shininess => (%.2f)\n", temp[0]);

    fprintf(gpFile, "[Initialize] [Lighting] isEnabled => (%hhd)\n", glIsEnabled(GL_LIGHTING));

    for (UInt32 idx = 0U; idx < 8; idx++)
    {
        fprintf(gpFile, "[Initialize] [Light%d] isEnabled => (%hhd)\n", idx, glIsEnabled(GL_LIGHT0 + idx));

        glGetLightfv(GL_LIGHT0 + idx, GL_AMBIENT, temp);
        fprintf(gpFile, "[Initialize] [Light%d] Ambient => (%.2f, %.2f, %.2f, %.2f)\n", idx, temp[0], temp[1], temp[2], temp[3]);

        glGetLightfv(GL_LIGHT0 + idx, GL_DIFFUSE, temp);
        fprintf(gpFile, "[Initialize] [Light%d] Diffuse => (%.2f, %.2f, %.2f, %.2f)\n", idx, temp[0], temp[1], temp[2], temp[3]);

        glGetLightfv(GL_LIGHT0 + idx, GL_SPECULAR, temp);
        fprintf(gpFile, "[Initialize] [Light%d] Specular => (%.2f, %.2f, %.2f, %.2f)\n", idx, temp[0], temp[1], temp[2], temp[3]);

        glGetLightfv(GL_LIGHT0 + idx, GL_POSITION, temp);
        fprintf(gpFile, "[Initialize] [Light%d] Position => (%.2f, %.2f, %.2f, %.2f)\n", idx, temp[0], temp[1], temp[2], temp[3]);

        glGetLightfv(GL_LIGHT0 + idx, GL_SPOT_DIRECTION, temp);
        fprintf(gpFile, "[Initialize] [Light%d] Spot Direction => (%.2f, %.2f, %.2f, %.2f)\n", idx, temp[0], temp[1], temp[2], temp[3]);

        glGetLightfv(GL_LIGHT0 + idx, GL_SPOT_CUTOFF, temp);
        fprintf(gpFile, "[Initialize] [Light%d] Spot Cutoff => (%.2f)\n", idx, temp[0]);

        glGetLightfv(GL_LIGHT0 + idx, GL_SPOT_EXPONENT, temp);
        fprintf(gpFile, "[Initialize] [Light%d] Spot Exponent => (%.2f)\n\n", idx, temp[0]);
    }


}
