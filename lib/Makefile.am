INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkworkspace.la

libkworkspace_la_SOURCES = kworkspace.cpp

libkworkspace_la_LDFLAGS = $(all_libraries) -version-info 1:0:0
libkworkspace_la_LIBADD  = $(LIB_KDECORE)
    

METASOURCES = AUTO

kworkspaceincludedir = $(includedir)/libkworkspace
kworkspaceinclude_HEADERS = kworkspace.h


config.lo: configbase.h
