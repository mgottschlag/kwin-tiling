kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(taskmanager_LIB_SRCS tasklmbmenu.cpp taskrmbmenu.cpp taskmanager.cpp )

kde4_automoc(${taskmanager_LIB_SRCS})

kde4_add_library(taskmanager SHARED ${taskmanager_LIB_SRCS})

target_link_libraries(taskmanager  ${QT_AND_KDECORE_LIBS} kickermain )

set_target_properties(taskmanager PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} taskmanager )


########### install files ###############


kde4_footer()



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
