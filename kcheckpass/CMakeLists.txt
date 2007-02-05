include_directories( ${UNIXAUTH_INCLUDE_DIRS} )

project(kcheckpass C)

set(kcheckpass_SRCS
	kcheckpass.h
	kcheckpass.c
	checkpass_etcpasswd.c
	checkpass_pam.c
	checkpass_shadow.c
	checkpass_osfc2passwd.c
	checkpass_aix.c
)

kde4_add_executable(kcheckpass NOGUI ${kcheckpass_SRCS})

if (KDE4_ENABLE_FPIE)
	macro_add_compile_flags(kcheckpass ${KDE4_CXX_FPIE_FLAGS})
	macro_add_link_flags(kcheckpass ${KDE4_PIE_LDFLAGS})
endif (KDE4_ENABLE_FPIE)

macro_add_compile_flags(kcheckpass -U_REENTRANT)
target_link_libraries(kcheckpass ${KDE4_KDEFAKES_LIBS} ${UNIXAUTH_LIBRARIES} ${SOCKET_LIBRARIES})
install(TARGETS kcheckpass DESTINATION ${BIN_INSTALL_DIR} PERMISSIONS SETUID OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

install_pam_service(kcheckpass)

#EXTRA_DIST = README
