
add_subdirectory( info ) 
add_subdirectory( ioslaveinfo ) 
add_subdirectory( nics ) 
add_subdirectory( pics ) 
add_subdirectory( usbview ) 
macro_optional_find_package(RAW1394)

if(RAW1394_FOUND)
   add_subdirectory( view1394 )
endif(RAW1394_FOUND)
