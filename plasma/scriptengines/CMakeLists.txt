add_subdirectory(webkit)
add_subdirectory(qscript)


if(GOOGLEGADGETS_FOUND)
   # Laurent "ggadget/floating_main_view_decorator.h"  is not in lib version 0.10.1 just in trunk
   # remove it when we used new lib (0.10.2 ?)
   FIND_PATH(GOOGLE_DECORATOR_PATH ggadget/floating_main_view_decorator.h PATH ${GOOGLEGADGETS_INCLUDE_DIRS})
   if( GOOGLE_DECORATOR_PATH )
      macro_optional_add_subdirectory(google_gadgets)
   else (GOOGLE_DECORATOR_PATH)
      MESSAGE(STATUS "You google lib is too old. Use trunk please")
   endif( GOOGLE_DECORATOR_PATH )
endif(GOOGLEGADGETS_FOUND)

