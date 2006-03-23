kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kwebdesktop_SRCS kwebdesktop.cpp )

kde4_automoc(${kwebdesktop_SRCS})

kde4_add_kcfg_files(kwebdesktop_SRCS kwebdesktopsettings.kcfgc )

kde4_add_executable(kwebdesktop ${kwebdesktop_SRCS})

target_link_libraries(kwebdesktop  ${KDE4_KPARTS_LIBS} khtml )

install_targets(/bin kwebdesktop )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES kwebdesktop.kcfg  )
install_files( ${DATA_INSTALL_DIR}/kdesktop/programs FILES kwebdesktop.desktop )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES= $(all_includes)
#LDADD = $(LIB_KHTML)
#
#bin_PROGRAMS = 	kwebdesktop
#
#METASOURCES = AUTO
#
#kwebdesktop_SOURCES = kwebdesktop.cpp kwebdesktopsettings.kcfgc 
#kwebdesktop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#kdesktop_kwebdesktop_data_DATA = kwebdesktop.desktop
#kdesktop_kwebdesktop_datadir = $(kde_datadir)/kdesktop/programs
#
#kde_kcfg_DATA = kwebdesktop.kcfg 
