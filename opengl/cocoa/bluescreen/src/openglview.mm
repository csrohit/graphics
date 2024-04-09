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

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                               CVOptionFlags, CVOptionFlags*, void*);
extern FILE *gpFile;

@implementation MyOpenGLView {
    @private
        CVDisplayLinkRef displayLink;
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
        [self display];
        [self updateScene];
        [pool release];

        return (kCVReturnSuccess);
    }

    - (void)dealloc {
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
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

        fprintf(gpFile, "OpenGL Info\n");
        fprintf(gpFile, "Vendor : %s\n", glGetString(GL_VENDOR));
        fprintf(gpFile, "Renderer : %s\n", glGetString(GL_RENDERER));
        fprintf(gpFile, "Version : %s\n", glGetString(GL_VERSION));
        fprintf(gpFile, "GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

        [self reshape];
    }

    -(BOOL)acceptsFirstResponder
    {
        [[self window] makeFirstResponder:self];
        return (YES);
    }

    -(void)keyDown:(NSEvent *)event {
        int key = [[event characters] characterAtIndex:0];

        switch(key)
        {
            case 27:
                [[self window] close];
                break;
            
            case 'F':
            case 'f':
                [[self window] toggleFullScreen:self];
                break;

            }
        }

    - (void) display
    {
        [[self openGLContext] makeCurrentContext];
        CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        
        /*--- draw here ---*/
        glClear(GL_COLOR_BUFFER_BIT);
        /*--- ** ---*/
        glFlush();
        CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
        CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    }


    - (void)updateScene
    {
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
    
    gluLookAt(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

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

