/********************************************************************************
* LinearPhysics physics library, http://www.linearphysics.com                 *
* Copyright (c) 2010-2016 Daniel Chappuis                                       *
*********************************************************************************
*                                                                               *
* This software is provided 'as-is', without any express or implied warranty.   *
* In no event will the authors be held liable for any damages arising from the  *
* use of this software.                                                         *
*                                                                               *
* Permission is granted to anyone to use this software for any purpose,         *
* including commercial applications, and to alter it and redistribute it        *
* freely, subject to the following restrictions:                                *
*                                                                               *
* 1. The origin of this software must not be misrepresented; you must not claim *
*    that you wrote the original software. If you use this software in a        *
*    product, an acknowledgment in the product documentation would be           *
*    appreciated but is not required.                                           *
*                                                                               *
* 2. Altered source versions must be plainly marked as such, and must not be    *
*    misrepresented as being the original software.                             *
*                                                                               *
* 3. This notice may not be removed or altered from any source distribution.    *
*                                                                               *
********************************************************************************/

// Libraries
#include "TestbedApplication.h"
#include "nanogui/nanogui.h"
#include <LINEARWIN/linearwin.h>

using namespace nanogui;

// LINEARWIN
//
#if defined(NANOGUI_USE_OPENGL)
#  if defined(NANOGUI_GLAD)
#    if defined(NANOGUI_SHARED) && !defined(LINEARGL_GLAPI_EXPORT)
#      define LINEARGL_GLAPI_EXPORT
#    endif
#    include <lineargl/lineargl.h>
#  else
#    if defined(__APPLE__)
#      define LINEARWIN_INCLUDE_GLCOREARB
#    else
#      define GL_GLEXT_PROTOTYPES
#    endif
#  endif
#elif defined(NANOGUI_USE_GLES)
#  define LINEARWIN_INCLUDE_ES2
#endif

// Main function
int main(int /*argc*/, char** /*argv*/) {

    // Create and start the testbed application
    TestbedApplication application;
    application.start();

    return 0;
}
