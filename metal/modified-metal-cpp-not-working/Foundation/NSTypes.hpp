//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Foundation/NSTypes.hpp
//
// Copyright 2020-2021 Apple Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "NSDefines.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <cstdint>

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace NS
{
using TimeInterval = double;

using Integer                    = std::intptr_t;
using UInteger                   = std::uintptr_t;

const Integer  IntegerMax  = INTPTR_MAX;
const Integer  IntegerMin  = INTPTR_MIN;
const UInteger UIntegerMax = UINTPTR_MAX;

struct OperatingSystemVersion
{
    Integer majorVersion;
    Integer minorVersion;
    Integer patchVersion;
} _NS_PACKED;

/*-- Rohit Nimkar: Begin --*/
using OpenGLPixelFormatAttribute = uint32_t;
_NS_ENUM(uint32_t, OpenGLPFA){
    /* Choose from all available renderers. */
    OpenGLPFAAllRenderers = 1,
    /* Choose a triple buffered pixel format. */
    OpenGLPFATripleBuffer = 3,
    /* Choose a double buffered pixel format. */
    OpenGLPFADoubleBuffer = 5,
    /* Number of aux buffers. */
    OpenGLPFAAuxBuffers = 7,
    /* Number of color buffer bits. */
    OpenGLPFAColorSize = 8,
    /* Number of alpha component bits. */
    OpenGLPFAAlphaSize = 11,
    /* Number of depth buffer bits. */
    OpenGLPFADepthSize = 12,
    /* Number of stencil buffer bits. */
    OpenGLPFAStencilSize = 13,
    /* Number of accum buffer bits. */
    OpenGLPFAAccumSize = 14,
    /* Never choose smaller buffers than requested. */
    OpenGLPFAMinimumPolicy = 51,
    /* Choose largest buffers of type requested. */
    OpenGLPFAMaximumPolicy = 52,
    /* Number of multi sample buffers. */
    OpenGLPFASampleBuffers = 55,
    /* Number of samples per multi sample buffer. */
    OpenGLPFASamples = 56,
    /* Each aux buffer has its own depth stencil. */
    OpenGLPFAAuxDepthStencil = 57,
    /* Color buffers store floating point pixels. */
    OpenGLPFAColorFloat = 58,
    /* Choose multisampling. */
    OpenGLPFAMultisample = 59,
    /* Choose supersampling. */
    OpenGLPFASupersample = 60,
    /* Request alpha filtering. */
    OpenGLPFASampleAlpha = 61,
    /* Request renderer by ID. */
    OpenGLPFARendererID = 70,
    /* Disable all failure recovery systems. */
    OpenGLPFANoRecovery = 72,
    /* Choose a hardware accelerated renderer. */
    OpenGLPFAAccelerated = 73,
    /* Choose the closest color buffer to request. */
    OpenGLPFAClosestPolicy = 74,
    /* Back buffer contents are valid after swap. */
    OpenGLPFABackingStore = 76,
    /* Bit mask of supported physical screens. */
    OpenGLPFAScreenMask = 84,
    /* Allow use of offline renderers. */
    OpenGLPFAAllowOfflineRenderers = 96,
    /* Choose a hardware accelerated compute device. */
    OpenGLPFAAcceleratedCompute = 97,
    /* Specify an OpenGL Profile to use. */
    OpenGLPFAOpenGLProfile = 99,
    /* Number of virtual screens in this format. */
    OpenGLPFAVirtualScreenCount = 128,

    /** These are even more deprecated than the regular OpenGL deprecation. */

    OpenGLPFAStereo = 6,
    OpenGLPFAOffScreen = 53,
    OpenGLPFAFullScreen = 54,
    OpenGLPFASingleRenderer = 71,
    OpenGLPFARobust = 75,
    OpenGLPFAMPSafe = 78,
    OpenGLPFAWindow = 80,
    OpenGLPFAMultiScreen = 81,
    OpenGLPFACompliant = 83,
    OpenGLPFAPixelBuffer = 90,
    OpenGLPFARemotePixelBuffer = 91,
};

_NS_ENUM(uint32_t, OpenGLProfileVersion){
    /* Choose a Legacy/Pre-OpenGL 3.0 Implementation. */
    OpenGLProfileVersionLegacy = 0x1000,
    /* Choose an OpenGL 3.2 Core Implementation. */
    OpenGLProfileVersion3_2Core = 0x3200,
    /* Choose an OpenGL 4.1 Core Implementation. */
    OpenGLProfileVersion4_1Core = 0x4100,
};

/*-- Rohit Nimkar: End --*/
} // namespace NS

//-------------------------------------------------------------------------------------------------------------------------------------------------------------
