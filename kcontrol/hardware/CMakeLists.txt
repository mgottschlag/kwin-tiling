kde4_header()

add_subdirectory( display ) 

if (CMAKE_SYSTEM_NAME MATCHES Linux)
   # this one doesn't seem to be very portable, Alex
   add_subdirectory( joystick ) 
endif (CMAKE_SYSTEM_NAME MATCHES Linux)

kde4_footer()

