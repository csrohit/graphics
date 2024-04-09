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

struct MyLight
{
    int iLight;
    bool state;
    GLfloat* currentColor;
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat position[4];
    GLfloat axis[3];
};

GLfloat black[4]               = {0.0f, 0.0f, 0.0f, 1.0f};
struct MyLight lights[]= 
{
    {
        GL_LIGHT0,
        false,
        black,
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {2.0f, 0.0f, 0.0f,1.0f},
        {0.0f, 1.0f, 0.0f},
    },
    {
        GL_LIGHT1,
        false,
        black,
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 2.0f, 0.0f,1.0f},
        {0.0f, 0.0f, 1.0f},
    },
    {
        GL_LIGHT2,
        false,
        black,
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 2.0f,1.0f},
        {1.0f, 0.0f, 0.0f},
    },
    {
        GL_LIGHT3,
        false,
        black,
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 2.0f,1.0f},
        {1.0f, 1.0f, 0.0f},
    },
    {
        GL_LIGHT4,
        false,
        black,
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {2.0f, 0.0f, 0.0f,1.0f},
        {0.0f, 1.0f, 1.0f},
    },
    {
        GL_LIGHT5,
        false,
        black,
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 2.0f, 0.0f,1.0f},
        {1.0f, 0.0f, 1.0f},
    },

};

GLfloat globalAmbient[]        = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat ground[]               = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat cosmisAmbientDefault[] = {0.2f, 0.2f, 0.2f, 1.0f};

GLfloat materialAmbient[]      = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialDiffuse[]      = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[]     = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess      = 128.0f;


/* CVDisplayCallback */
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                               CVOptionFlags, CVOptionFlags*, void*);
extern FILE *gpFile;

@implementation MyOpenGLView {
    @private
        GLfloat angleRed;
        GLfloat angleGreen;
        GLfloat angleBlue;
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
        [super initWithFrame:frameRect];
        [super setPixelFormat: [pixelformat autorelease]];
        return self;
        return self = [ super initWithFrame: frameRect pixelFormat: [ pixelformat autorelease ] ];
    }

    -(CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime
    {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        [self display];
        [self updateScene];
        [pool release];

        return (kCVReturnSuccess);
    }

    - (void)dealloc {
        if (nullptr != pQuadric)
        {
            gluDeleteQuadric(pQuadric);
            pQuadric = nullptr;
        }

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
        glShadeModel(GL_SMOOTH);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        
        struct MyLight* pLight;
        //set up light properties
        for(uint32_t idx = 0U; idx < sizeof(lights)/sizeof(struct MyLight); idx++)
        {
            pLight = lights + idx;
            glLightfv(pLight->iLight, GL_AMBIENT, pLight->ambient);
            glLightfv(pLight->iLight, GL_DIFFUSE, pLight->diffuse);
            glLightfv(pLight->iLight, GL_SPECULAR, pLight->specular);
            glLightfv(pLight->iLight, GL_POSITION, pLight->position);
            //glEnable(GL_LIGHT0 + idx);
        }
        fprintf(gpFile, "OpenGL Info\n");
        fprintf(gpFile, "Vendor : %s\n", glGetString(GL_VENDOR));
        fprintf(gpFile, "Renderer : %s\n", glGetString(GL_RENDERER));
        fprintf(gpFile, "Version : %s\n", glGetString(GL_VERSION));
        fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
        //set up material properties
        glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, materialShininess);

        //glEnable(GL_LIGHTING);

        /* Initialize quadric */
        pQuadric = gluNewQuadric();
        [self reshape];
    }

    -(BOOL)acceptsFirstResponder
    {
        [[self window] makeFirstResponder:self];
        return (YES);
    }

    -(void)keyDown:(NSEvent *)event {
        int key = [[event characters] characterAtIndex:0];

        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);

        if(( key >= '0' )&& (key - '0' < sizeof(lights)/sizeof(struct MyLight)))
        {
            uint32_t idx = key - '0';
            struct MyLight* pLight = lights + idx;
            if(false == pLight->state)
            {
                [self toggleLighting];
                glEnable(pLight->iLight);
                pLight->currentColor = pLight->diffuse;
                [self toggleLighting];
            }
            else
            {    
                [self toggleLighting];
                glDisable(pLight->iLight);
                pLight->currentColor = black;
                [self toggleLighting];
            }
            pLight->state = !pLight->state;
        }
        else
        {
            switch(key)
            {
                case 27:
                    [[self window] close];
                    break;
                
                case 'F':
                case 'f':
                    [[self window] toggleFullScreen:self];
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

        glColor3f(1.0f, 1.0f, 1.0f);
 
        glLoadIdentity();

        glMaterialfv(GL_FRONT, GL_EMISSION, black);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
        gluSphere(pQuadric, 0.5f, 100, 100); // it will create all normals for you
        
        glMaterialfv(GL_FRONT, GL_DIFFUSE, black);
        glMaterialfv(GL_FRONT, GL_SPECULAR, black);
        struct MyLight* pLight;
        for(uint32_t idx = 0U; idx < sizeof(lights)/sizeof(struct MyLight); idx++)
        {
            pLight = lights + idx;
            glPushMatrix();
            glRotatef(angleRed, pLight->axis[0], pLight->axis[1], pLight->axis[2]);
            glTranslatef(pLight->position[0], pLight->position[1],pLight->position[2]);
            glLightfv(pLight->iLight, GL_POSITION, black);
            glMaterialfv(GL_FRONT, GL_EMISSION, pLight->currentColor);
            gluSphere(pQuadric, 0.1f, 20, 20); // it will create all normals for you
            glPopMatrix();
        }

        /*--- ** ---*/
        glFlush();
        CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
        CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    }


    - (void)updateScene
    {
        angleRed += 0.5f;
        if(angleRed > 360.0f)
        {
            angleRed -= 360.0f;
        }
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
    
    gluLookAt(0.0, 2.0, 6.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

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

