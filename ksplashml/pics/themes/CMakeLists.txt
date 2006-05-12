include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )
add_subdirectory(blue-bend)
add_subdirectory(default)
add_subdirectory(none)
add_subdirectory(simple)

########### install files ###############





#original Makefile.am contents follow:

#SUBDIRS=$(AUTODIRS)
#
