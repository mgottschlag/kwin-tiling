include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib/ )

ADD_DEFINITIONS( -DKSYSGUARDDRCFILE=\"\\"${CMAKE_INSTALL_PREFIX}${SYSCONF_INSTALL_DIR}/kde-system-monitor-daemonrc\\"\" )

ADD_DEFINITIONS(-DOSTYPE_${CMAKE_SYSTEM_NAME})

macro_optional_find_package(Sensors)
macro_bool_to_01(SENSORS_FOUND, HAVE_LMSENSORS)
FIND_LIBRARY(XRES_LIBRARY XRes)
if(XRES_LIBRARY)
  set(HAVE_XRES 1)
endif(XRES_LIBRARY)
configure_file(config-kde-system-monitor-daemon.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde-system-monitor-daemon.h)

add_subdirectory( ${CMAKE_SYSTEM_NAME} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME} )


########### next target ###############

set(libccont_SRCS 
	${CMAKE_CURRENT_SOURCE_DIR}/../CContLib/ccont.c )

set(kde-system-monitor-daemon_SRCS ${libccont_SRCS}
   Command.c 
   conf.c 
   kde-system-monitor-daemon.c 
   PWUIDCache.c )

kde4_automoc(${kde-system-monitor-daemon_SRCS})

kde4_add_executable(kde-system-monitor-daemon ${kde-system-monitor-daemon_SRCS})

# was kdefakes_nonpic...
target_link_libraries(kde-system-monitor-daemon  ${KDE4_KDECORE_LIBS} kdefakes )

ADD_DEPENDENCIES(kde-system-monitor-daemon libkde-system-monitor-daemon)
TARGET_LINK_LIBRARIES(kde-system-monitor-daemon libkde-system-monitor-daemon)

install(TARGETS kde-system-monitor-daemon DESTINATION bin)


########### install files ###############





#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = foreign
#
#if include_kde-system-monitor-daemon_linux
#linux_SUBDIR = Linux
#endif
#if include_kde-system-monitor-daemon_freebsd
#freebsd_SUBDIR = FreeBSD
#endif
#if include_kde-system-monitor-daemon_netbsd
#netbsd_SUBDIR = NetBSD
#endif
#if include_kde-system-monitor-daemon_solaris
#solaris_SUBDIR = Solaris
#endif
#if include_kde-system-monitor-daemon_tru64
#tru64_SUBDIR = Tru64
#endif
#if include_kde-system-monitor-daemon_irix
#irix_SUBDIR = Irix
#endif
#if include_kde-system-monitor-daemon_openbsd
#openbsd_SUBDIR = OpenBSD
#endif
#
#SUBDIRS = $(linux_SUBDIR) $(freebsd_SUBDIR) $(netbsd_SUBDIR)\
#	$(solaris_SUBDIR) $(tru64_SUBDIR) $(irix_SUBDIR) $(openbsd_SUBDIR)
#
#KSYSGUARDDRCFILE=$(sysconfdir)/kde-system-monitor-daemonrc
#INCLUDES = -DKSYSGUARDDRCFILE="\"$(KSYSGUARDDRCFILE)\"" -DOSTYPE_$(UNAME) -I$(srcdir)/../CContLib -I$(srcdir)/$(UNAME)
#
#bin_PROGRAMS = kde-system-monitor-daemon
#
#kde-system-monitor-daemon_SOURCES = Command.c conf.c kde-system-monitor-daemon.c PWUIDCache.c
#kde-system-monitor-daemon_LDFLAGS = $(all_libraries)
#kde-system-monitor-daemon_LDADD =  $(top_builddir)/workspace/ksysguard/kde-system-monitor-daemon/$(UNAME)/libkde-system-monitor-daemon.a ../CContLib/libccont.a -lkdefakes_nonpic $(LIBHOSTS)
