kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kdmlib ${CMAKE_SOURCE_DIR}/workspace/kdesktop ${CMAKE_SOURCE_DIR}/workspace/lib ${CMAKE_SOURCE_DIR}/kcheckpass ${CMAKE_SOURCE_DIR}/kdmlib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdesktop_lock_SRCS ${kdmlib_SRCS}
   lockprocess.cc 
   lockdlg.cc 
   autologout.cc 
   main.cc )

kde4_add_kcfg_files(kdesktop_lock_SRCS 
	${CMAKE_SOURCE_DIR}/workspace/kdesktop/kdesktopsettings.kcfgc
	${CMAKE_SOURCE_DIR}/workspace/kdesktop/klaunchsettings.kcfgc )


kde4_automoc(${kdesktop_lock_SRCS})

kde4_add_executable(kdesktop_lock ${kdesktop_lock_SRCS} )

target_link_libraries(kdesktop_lock  ${LIB_KIO} )

install_targets(/bin kdesktop_lock )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

### Makefile.am of kdebase/kdesktop/lock
#
#INCLUDES = -I.. -I$(top_srcdir)/workspace/lib -I$(top_srcdir)/kcheckpass -I$(top_srcdir)/kdmlib $(GLINC) $(all_includes)
#kdesktop_lock_LDFLAGS  = $(all_libraries) $(KDE_RPATH)
#kdesktop_lock_LDADD    = ../libkdesktopsettings.la ../../../kdmlib/libdmctl.la $(LIB_KIO) $(LIB_XF86MISC) $(GLLIB)
#
######## Files
#
#bin_PROGRAMS = kdesktop_lock
#
#kdesktop_lock_SOURCES = lockprocess.cc lockdlg.cc autologout.cc main.cc
#
#noinst_HEADERS = lockprocess.h lockdlg.h autologout.h main.h
#
#METASOURCES = AUTO
#
#lockprocess.o: ../kdesktopsettings.h
#
######## Build rules
#
#PAM = @KSCREENSAVER_PAM_SERVICE@
#
#install-data-local:
#	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || $(top_srcdir)/mkpamserv $(PAM)
