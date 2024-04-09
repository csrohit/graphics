/**
 * @file        appdelegate.mm
 * @description Application and Window event handler
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

#import "appdelegate.h"
#import "openglview.h"
#include "bridge.h"

FILE* gpFile;
@implementation AppDelegate
{
    @private
        NSWindow *window;
        MyOpenGLView *openGLView;
}
    
    -(void)applicationDidFinishLaunching:(NSNotification*)aNotification
    {
        /* open file for logging */
        //gpFile = fopen("log.txt", "w");
        gpFile = stdout;
        if(gpFile == NULL)
        {
            [self release];
            [NSApp terminate:self];
        }
        fprintf(gpFile, "Application is starting\n");
        
        /* create window */
        window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0.0, 0.0, WIN_WIDTH, WIN_HEIGHT)
            styleMask:NSWindowStyleMaskTitled |
                      NSWindowStyleMaskClosable |
                      NSWindowStyleMaskMiniaturizable |
                      NSWindowStyleMaskResizable
            backing:NSBackingStoreBuffered
            defer:NO
        ];

        /* Make window appear in the center */
        [window setTitle:@WINDOW_TITLE];
        [window center];

        /* Initialize openGLView */
        openGLView = [[MyOpenGLView alloc] initWithFrame:window.contentView.frame];

        /* Set oepnglview as current view to current window */
        [window setContentView:openGLView];

        /* set window delegate to handle window events events */
        [window setDelegate:self];

        /* Make window visible on screen */
        [window makeKeyAndOrderFront:openGLView];

        /* Default activation policy for current app is NSApplicationActivationPolicyProhibited
            Changing it to make our application have GUI and dock tile
            This will ensure our app gets the events
        */
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        [NSApp activateIgnoringOtherApps:true];
    }

    -(void)applicationWillTerminate:(NSNotification*)aNotification
    {
        fprintf(gpFile, "application is about to terminate\n");
        if(nullptr != gpFile)
        {
            fclose(gpFile);
            gpFile = nullptr;
        }
    }

    -(void)windowWillClose:(NSNotification*)aNotification
    {
        fprintf(gpFile, "Stopping message loop\n");
        [NSApp stop:self];
    }

    - (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
        return true;
    }

    -(void)dealloc
    {
        [openGLView release];
        [window release];
        [super dealloc];
    }

@end


