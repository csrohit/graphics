#pragma once

#include "../Foundation/NSDefines.hpp"
#include "../Foundation/NSObject.hpp"
#include "../Foundation/NSPrivate.hpp"
#include "../Foundation/NSTypes.hpp"

namespace NS
{
class OpenGLPixelFormat : public NS::Referencing<OpenGLPixelFormat>
{
  public:
    static OpenGLPixelFormat *alloc();
    OpenGLPixelFormat        *init(OpenGLPixelFormatAttribute *attributes);
};
} // namespace NS

_NS_INLINE NS::OpenGLPixelFormat *NS::OpenGLPixelFormat::alloc()
{
    return NS::Object::sendMessage<NS::OpenGLPixelFormat *>(_NS_PRIVATE_CLS(NSOpenGLPixelFormat), _NS_PRIVATE_SEL(alloc));
}

_NS_INLINE NS::OpenGLPixelFormat *NS::OpenGLPixelFormat::init(OpenGLPixelFormatAttribute *attributes)
{
    return NS::Object::sendMessage<NS::OpenGLPixelFormat *>(this, _NS_PRIVATE_SEL(initWithAttributes_), attributes);
}
