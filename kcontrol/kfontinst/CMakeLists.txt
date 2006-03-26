kde4_header()

add_subdirectory( lib ) 
add_subdirectory( kcmfontinst ) 
add_subdirectory( kfile-plugin ) 
add_subdirectory( kfontinst ) 
add_subdirectory( kio ) 

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(FONTINST_TN_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(FONTINST_VP_SUBDIR)")
include_directories( ${KDE4_INCLUDES} )


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
