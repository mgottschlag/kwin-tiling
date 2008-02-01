
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( info ) 
add_subdirectory( nics ) 
add_subdirectory( usbview ) 
macro_optional_find_package(RAW1394)

if(RAW1394_FOUND)
   add_subdirectory( view1394 )
endif(RAW1394_FOUND)
