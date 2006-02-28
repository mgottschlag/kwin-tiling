kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcheckpass_SRCS 
   kcheckpass.c 
   checkpass_etcpasswd.c 
   checkpass_pam.c 
   checkpass_shadow.c 
   checkpass_osfc2passwd.c 
   checkpass_aix.c )

kde4_automoc(${kcheckpass_SRCS})

kde4_add_executable(kcheckpass ${kcheckpass_SRCS})

target_link_libraries(kcheckpass  ${QT_AND_KDECORE_LIBS} kdefakes )

install_targets(/bin kcheckpass )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for kcheckpass
### written by Christian Esken
###
#
#INCLUDES= $(KDE_USE_FPIE) $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
#
#bin_PROGRAMS = kcheckpass
#
#kcheckpass_SOURCES = kcheckpass.c \
#	checkpass_etcpasswd.c checkpass_pam.c checkpass_shadow.c \
#	checkpass_osfc2passwd.c checkpass_aix.c
#kcheckpass_LDADD = -lkdefakes $(PASSWDLIBS) $(LIBSOCKET)
#kcheckpass_LDFLAGS = $(KDE_USE_PIE) $(all_libraries)
#
#noinst_HEADERS = kcheckpass.h
#
#EXTRA_DIST = README
#
#PAM = $(KCHECKPASS_PAM_SERVICE)
#
#install-data-local:
#	-@test -n "$(DESTDIR)" || test -z "$(PAM)" || $(top_srcdir)/mkpamserv $(PAM)
#
#install-exec-hook:
#	@(chown 0 $(DESTDIR)$(bindir)/kcheckpass && chmod 4755 $(DESTDIR)$(bindir)/kcheckpass) \
#	|| echo "Error: Could not install kcheckpass as setuid root (possibly you won't be able to unlock)!!"
