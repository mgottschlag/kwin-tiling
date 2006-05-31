


########### next target ###############

set(taskmanager_LIB_SRCS tasklmbmenu.cpp taskrmbmenu.cpp taskmanager.cpp )

kde4_automoc(${taskmanager_LIB_SRCS})

kde4_add_library(taskmanager SHARED ${taskmanager_LIB_SRCS})

target_link_libraries(taskmanager  ${KDE4_KDECORE_LIBS} kickermain )
if (X11_Xfixes_FOUND)
  target_link_libraries(taskmanager ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)
if (X11_Xrender_FOUND)
  target_link_libraries(taskmanager ${X11_Xrender_LIB})
endif (X11_Xrender_FOUND)
if (X11_Xcomposite_FOUND)
  target_link_libraries(taskmanager ${X11_Xcomposite_LIB})
endif (X11_Xcomposite_FOUND)

set_target_properties(taskmanager PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} taskmanager )


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I$(srcdir)/../libkicker
#
#lib_LTLIBRARIES = libtaskmanager.la
#libtaskmanager_la_SOURCES = tasklmbmenu.cpp taskrmbmenu.cpp taskmanager.cpp
#libtaskmanager_la_METASOURCES = AUTO
#
#libtaskmanager_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
#libtaskmanager_la_LIBADD = $(LIB_KDECORE) $(LIB_XFIXES) $(LIB_XRENDER) $(LIB_XCOMPOSITE) ../libkicker/libkickermain.la
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/libtaskmanager.pot
