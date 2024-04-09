#include <iostream>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>

// Forward declare Objective-C classes to be used
@class NSApplication;
@class NSWindow;

extern "C" {
    // Objective-C methods
    void NSApplicationLoad(void);
    NSApplication* NSApp();
    NSWindow* NSApp_window();
}

int main() {
    NSApplicationLoad(); // Load the NSApplication

    // Create an autorelease pool to manage memory
    void* pool = reinterpret_cast<void*>(objc_msgSend((id)objc_getClass("NSAutoreleasePool"), sel_registerName("new")));

    // Get the shared application instance
    NSApplication* app = NSApp();

    // Create a window
    NSWindow* window = NSApp_window();

    // Set window title
    objc_msgSend((id)window, sel_registerName("setTitle:"), "C++ Window");

    // Show the window
    objc_msgSend((id)window, sel_registerName("makeKeyAndOrderFront:"), nil);

    // Run the application event loop
    objc_msgSend((id)app, sel_registerName("run"));

    // Release the autorelease pool
    objc_msgSend(pool, sel_registerName("release"));

    return 0;
}

