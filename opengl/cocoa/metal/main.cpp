#include <iostream>
#include "AppKit/AppKit.hpp"
#include <AppKit/NSOpenGLView.hpp>
#include <AppKit/NSWindow.hpp>

#pragma region Declarations {
class AppDelegate: public NS::ApplicationDelegate
{
    public:
        ~AppDelegate();

        NS::Menu* createMenuBar();

        virtual void applicationWillFinishLaunching( NS::Notification* pNotification ) override;
        virtual void applicationDidFinishLaunching( NS::Notification* pNotification ) override;
        virtual bool applicationShouldTerminateAfterLastWindowClosed( NS::Application* pSender ) override;

    private:
        NS::Window* pWindow;
        NS::View* pOpenGLView;
};

class MyOpenGLView: public NS::OpenGLView
{
        public:
            ~MyOpenGLView();
        virtual void prepareOpenGL() override;
        private:
            
};

class MyWindowDelegate: public NS::WindowDelegate
{
        public:
        virtual void windowWillClose(const NS::Notification* pNotification) override;

};

#pragma endregion Declarations }


using std::cout;
int main()
{
    NS::AutoreleasePool* pAutoReleasePool = NS::AutoreleasePool::alloc()->init();
    
    cout << "Hello world\n";
    AppDelegate del;
    NS::Application* pSharedApplication = NS::Application::sharedApplication();
    pSharedApplication->setDelegate(&del);
    pSharedApplication->run();

    pAutoReleasePool->release();
    return (0);
}

#pragma mark - AppDelegate
#pragma region AppDelegate {
AppDelegate::~AppDelegate()
{
    std::cout << "Destructor called" << std::endl;
    this->pWindow->release();
}

void AppDelegate::applicationDidFinishLaunching(NS::Notification* pNotification)
{
    std::cout << "Application finished lauching" << std::endl;
    CGRect frame = (CGRect){ {100.0, 100.0}, {512.0, 512.0} };

    this->pWindow = NS::Window::alloc()->init(
        frame,
        NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled,
        NS::BackingStoreBuffered,
        false );

    this->pOpenGLView = NS::OpenGLView::alloc()->init(frame);
    NS::OpenGLPixelFormatAttribute attributes[] = {
        NS::OpenGLPFA::OpenGLPFAOpenGLProfile,
        NS::OpenGLProfileVersionLegacy,
        NS::OpenGLPFAAccelerated,
        NS::OpenGLPFADoubleBuffer,
        NS::OpenGLPFAColorSize, 24,
        NS::OpenGLPFADepthSize, 24,
        NS::OpenGLPFAAlphaSize, 8,
        NS::OpenGLPFAMinimumPolicy,
        0
    };

    // NS::OpenGLPixelFormat *format = NS::OpenGLPixelFormat::alloc()->init(attributes);
    cout << "setting up pixel format view\n";
    // pOpenGLView->setPixelFormat(format);
    cout << "setting up content view\n";
    // pWindow->setContentView(pOpenGLView);
    cout << "done setting up content view\n";
    this->pWindow->setTitle( NS::String::string( "Rohit Nimkar: GL CPP", NS::StringEncoding::UTF8StringEncoding ) );
    this->pWindow->center();
    this->pWindow->makeKeyAndOrderFront( nullptr );

    NS::Application* pApp = reinterpret_cast< NS::Application* >( pNotification->object() );
    pApp->activateIgnoringOtherApps( true );
}

void AppDelegate::applicationWillFinishLaunching(NS::Notification* pNotification)
{
    std::cout << "Application will finish launching" << std::endl;
}

bool AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender)
{
    return true;

}

#pragma endregion AppDelegate }

#pragma mark -  OpenGLView
#pragma region AppDelegate }
void MyOpenGLView::prepareOpenGL()
{
    cout << "preparing openGL \n";
}
#pragma endregion AppDelegate }


#pragma mark -  MyWindowDelegate
#pragma region MyWindowDelegate }

void MyWindowDelegate::windowWillClose(const NS::Notification* pNotification)
{
    cout << "Window will be closeing from main.cpp\n";
}
#pragma endregion MyWindowDelegate }

