


########### next target ###############

set(kcm_info_PART_SRCS main.cpp memory.cpp opengl.cpp )

kde4_automoc(${kcm_info_PART_SRCS})

kde4_add_plugin(kcm_info ${kcm_info_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_info )

target_link_libraries(kcm_info  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${X11_X11_LIB})

install(TARGETS kcm_info  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES memory.desktop processor.desktop dma.desktop  	interrupts.desktop ioports.desktop opengl.desktop pci.desktop sound.desktop  	devices.desktop scsi.desktop partitions.desktop xserver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kcm_info.la
#
#if COMPILE_GL_INFO
#AM_CXXFLAGS = -DINFO_OPENGL_AVAILABLE
#endif
#
#kcm_info_la_SOURCES = main.cpp memory.cpp opengl.cpp
#
#kcm_info_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined $(FRAMEWORK_COREAUDIO)
#kcm_info_la_LIBADD = $(LIBALIB) $(LIB_KDEUI) $(LIB_TRU64_MACH) $(LIBCFG) $(LIBODM) $(LIBKSTAT) $(LIBDEVINFO) $(GLLIB)
#METASOURCES = AUTO
#
#AM_CPPFLAGS= $(all_includes) $(GLINC)
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcminfo.pot
#
#
#xdg_apps_DATA = memory.desktop processor.desktop dma.desktop \
#	interrupts.desktop ioports.desktop opengl.desktop pci.desktop sound.desktop \
#	devices.desktop scsi.desktop partitions.desktop xserver.desktop
