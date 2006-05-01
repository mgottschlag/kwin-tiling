kde_module_LTLIBRARIES = kcm_info.la

if COMPILE_GL_INFO
AM_CXXFLAGS = -DINFO_OPENGL_AVAILABLE
endif

kcm_info_la_SOURCES = main.cpp memory.cpp opengl.cpp

kcm_info_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined $(FRAMEWORK_COREAUDIO)
kcm_info_la_LIBADD = $(LIBALIB) $(LIB_KDEUI) $(LIB_TRU64_MACH) $(LIBCFG) $(LIBODM) $(LIBKSTAT) $(LIBDEVINFO) $(GLLIB)
METASOURCES = AUTO

AM_CPPFLAGS= $(all_includes) $(GLINC)



xdg_apps_DATA = memory.desktop processor.desktop dma.desktop \
	interrupts.desktop ioports.desktop opengl.desktop pci.desktop sound.desktop \
	devices.desktop scsi.desktop partitions.desktop xserver.desktop
