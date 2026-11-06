#pragma once

// Engine-internal: the single place that pulls in OpenGL. GLEW must be included
// before any other OpenGL header, so engine .cpp files include this instead of
// <GL/gl.h>. Game code never sees OpenGL directly.
#include <GL/glew.h>
