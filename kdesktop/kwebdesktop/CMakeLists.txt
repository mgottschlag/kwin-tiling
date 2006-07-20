


########### next target ###############

set(kwebdesktop_SRCS kwebdesktop.cpp )

kde4_automoc(${kwebdesktop_SRCS})

kde4_add_kcfg_files(kwebdesktop_SRCS kwebdesktopsettings.kcfgc )

kde4_add_executable(kwebdesktop ${kwebdesktop_SRCS})

target_link_libraries(kwebdesktop  ${KDE4_KPARTS_LIBS} khtml )

install(TARGETS kwebdesktop DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES kwebdesktop.kcfg   DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES kwebdesktop.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kdesktop/programs )




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
