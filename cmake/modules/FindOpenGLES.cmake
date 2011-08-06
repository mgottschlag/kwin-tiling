# - Try to find OpenGLES
# Once done this will define
#  
#  OPENGLES_FOUND           - system has OpenGLES and EGL
#  OPENGLES_INCLUDE_DIR     - the GLES include directory
#  OPENGLES_LIBRARY	    - the GLES library
#  OPENGLES_EGL_INCLUDE_DIR - the EGL include directory
#  OPENGLES_EGL_LIBRARY	    - the EGL library
#  OPENGLES_LIBRARIES       - all libraries needed for OpenGLES
#  OPENGLES_INCLUDES        - all includes needed for OpenGLES

FIND_PATH(OPENGLES_INCLUDE_DIR GLES2/gl2.h
  /usr/openwin/share/include
  /opt/graphics/OpenGL/include /usr/X11R6/include
  /usr/include
)

FIND_LIBRARY(OPENGLES_LIBRARY
  NAMES GLESv2
  PATHS /opt/graphics/OpenGL/lib
        /usr/openwin/lib
        /usr/shlib /usr/X11R6/lib
        /usr/lib
)

FIND_PATH(OPENGLES_EGL_INCLUDE_DIR EGL/egl.h
  /usr/openwin/share/include
  /opt/graphics/OpenGL/include /usr/X11R6/include
  /usr/include
)

FIND_LIBRARY(OPENGLES_EGL_LIBRARY
    NAMES EGL
    PATHS /usr/shlib /usr/X11R6/lib
          /usr/lib
)

# On Unix OpenGL most certainly always requires X11.
# Feel free to tighten up these conditions if you don't 
# think this is always true.
# It's not true on OSX.

IF(OPENGLES_LIBRARY)
  IF(NOT X11_FOUND)
    INCLUDE(FindX11)
  ENDIF(NOT X11_FOUND)
  IF(X11_FOUND)
    IF(NOT APPLE)
      SET(OPENGLES_LIBRARIES ${X11_LIBRARIES})
    ENDIF(NOT APPLE)
  ENDIF(X11_FOUND)
ENDIF(OPENGLES_LIBRARY)

SET(OPENGLES_FOUND "NO")
IF(OPENGLES_LIBRARY AND OPENGLES_INCLUDE_DIR AND
   OPENGLES_EGL_LIBRARY AND OPENGLES_EGL_INCLUDE_DIR)
    SET(OPENGLES_LIBRARIES ${OPENGLES_LIBRARY} ${OPENGLES_LIBRARIES}
                           ${OPENGLES_EGL_LIBRARY})
    SET(OPENGLES_INCLUDES ${OPENGLES_INCLUDE_DIR} ${OPENGLES_EGL_INCLUDE_DIR})
    SET(OPENGLES_FOUND "YES")
ENDIF()

