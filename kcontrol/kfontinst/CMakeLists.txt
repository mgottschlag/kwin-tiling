kde4_header()

if (X11_XFT_FOUND)

  add_definitions(${FREETYPE_INCLUDE_DIR})

  set(libkfontinstprint_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/KfiPrint.cpp )

  add_subdirectory( lib ) 
  add_subdirectory( kcmfontinst ) 
  add_subdirectory( kfile-plugin ) 
  add_subdirectory( kfontinst ) 
  add_subdirectory( kio ) 
  add_subdirectory(thumbnail)
  add_subdirectory(viewpart)

else (X11_XFT_FOUND)

  message(STATUS Missing Xft - disabling font installer and font previews)

endif (X11_XFT_FOUND)




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/konqueror/servicemenus FILES installfont.desktop )

kde4_footer()



#original Makefile.am contents follow:

#if include_fontinst_tn
#FONTINST_TN_SUBDIR=thumbnail
#FONTINST_VP_SUBDIR=viewpart
#endif
#
#SUBDIRS = lib $(FONTINST_TN_SUBDIR) $(FONTINST_VP_SUBDIR) kcmfontinst kfile-plugin kfontinst kio
#
#EXTRA_DIST = TODO
#
#konqservicedir = $(kde_datadir)/konqueror/servicemenus
#konqservice_DATA = installfont.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp */*.cpp */*.h -o $(podir)/kfontinst.pot
