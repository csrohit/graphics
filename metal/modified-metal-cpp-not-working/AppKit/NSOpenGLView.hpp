#pragma once

// #include "../Foundation/NSObject.hpp"
// #include "NSDefines.hpp"
// #include "NSOpenGLContext.hpp"
// #include "NSOpenGLPixelFormat.hpp"
// #include "NSPrivate.hpp"

#include "AppKit/NSOpenGLContext.hpp"
#include "AppKit/NSOpenGLPixelFormat.hpp"
#include "AppKit/NSWindow.hpp"
#include <AppKit/NSView.hpp>
#include <iostream>

namespace NS
{
class OpenGLView : public NS::Referencing<OpenGLView, NS::View>
{
  public:
    static OpenGLView *alloc();
    OpenGLView        *initWithFrame(CGRect frame);
    virtual void prepareOpenGL();

    OpenGLPixelFormat *pixelFormat();
    void               setPixelFormat(const OpenGLPixelFormat *pPixelFormat);

    OpenGLContext *openGLContext();
    void           setOpenGLContext(OpenGLContext *open_gl_context);

    bool wantsBestResolutionOpenGLSurface();
    void setWantsBestResolutionOpenGLSurface(bool value);
};
} // namespace NS

_NS_INLINE NS::OpenGLView *NS::OpenGLView::alloc()
{
    // std::cout << "setting prepare callback\n";

	// void (*prepareOpenGLDispatch)(NS::Value*, SEL) = []( Value* pSelf, SEL){
    //     auto pDel = reinterpret_cast<NS::OpenGLView *>(pSelf->pointerValue());
    //     std::cout <<"callback came back\n";
    //     pDel->prepareOpenGL();
	// };
    // class_addMethod((Class)objc_lookUpClass("NSValue"), sel_registerName("prepareOpenGL"), (IMP)prepareOpenGLDispatch, "v@");

    // return Object::sendMessage<OpenGLView *>(_NS_PRIVATE_CLS(NSOpenGLView), _NS_PRIVATE_SEL(alloc));

	return NS::Object::alloc< OpenGLView >( _NS_PRIVATE_CLS( NSOpenGLView ) );
}

_NS_INLINE NS::OpenGLView *NS::OpenGLView::initWithFrame(CGRect frame)
{
    return NS::Object::sendMessage<OpenGLView *>(this, _NS_PRIVATE_SEL(initWithFrame_), frame);
}

_NS_INLINE NS::OpenGLPixelFormat *NS::OpenGLView::pixelFormat()
{
    return NS::Object::sendMessage<NS::OpenGLPixelFormat *>(this, _NS_PRIVATE_SEL(pixelFormat));
}

_NS_INLINE void NS::OpenGLView::setPixelFormat(const NS::OpenGLPixelFormat *pPixelFormat)
{
    NS::Object::sendMessage<void>(this, _NS_PRIVATE_SEL(setPixelFormat_), pPixelFormat);
}

_NS_INLINE NS::OpenGLContext *NS::OpenGLView::openGLContext()
{
    return NS::Object::sendMessage<NS::OpenGLContext *>(this, _NS_PRIVATE_SEL(openGLContext));
}

_NS_INLINE void NS::OpenGLView::setOpenGLContext(NS::OpenGLContext *setOpenGLContext)
{
    NS::Object::sendMessage<void>(this, _NS_PRIVATE_SEL(setOpenGLContext_), setOpenGLContext);
}

_NS_INLINE void NS::OpenGLView::prepareOpenGL()
{
    return NS::Object::sendMessage<void>(this, _NS_PRIVATE_SEL(prepareOpenGL_));
}

_NS_INLINE bool NS::OpenGLView::wantsBestResolutionOpenGLSurface()
{
    return NS::Object::sendMessage<bool>(this, _NS_PRIVATE_SEL(wantsBestResolutionOpenGLSurface));
}

_NS_INLINE void NS::OpenGLView::setWantsBestResolutionOpenGLSurface(bool value)
{
    NS::Object::sendMessage<void>(this, _NS_PRIVATE_SEL(setWantsBestResolutionOpenGLSurface_), value);
}
