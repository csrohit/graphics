/**
 * @file        main.mm
 * @description Application entry point
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
#import "appdelegate.h"

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        /* Create application instance and set delegate */
        NSApplication *application = [NSApplication sharedApplication];
        AppDelegate *delegate = [[AppDelegate alloc] init];
        [application setDelegate:delegate];

        /* start application main loop */
        [application run];
    }
    return (0);
}
