kde4_header()

add_subdirectory( info ) 
add_subdirectory( ioslaveinfo ) 
add_subdirectory( nics ) 
add_subdirectory( pics ) 
add_subdirectory( usbview ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KCONTROLVIEW1394)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#if include_kcontrol_view1394
#KCONTROLVIEW1394 = view1394
#endif
#
#SUBDIRS = info  ioslaveinfo  nics  pics  usbview  $(KCONTROLVIEW1394)
