
add_subdirectory( info ) 
add_subdirectory( ioslaveinfo ) 
add_subdirectory( nics ) 
add_subdirectory( pics ) 
add_subdirectory( usbview ) 


macro_optional_find_package(RAW1394)

if(RAW1394_FOUND)
   add_subdirectory( view1394 )
endif(RAW1394_FOUND)



########### install files ###############





#original Makefile.am contents follow:

#if include_kcontrol_view1394
#KCONTROLVIEW1394 = view1394
#endif
#
#SUBDIRS = info  ioslaveinfo  nics  pics  usbview  $(KCONTROLVIEW1394)
