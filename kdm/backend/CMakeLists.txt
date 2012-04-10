set(kdm_SRCS
	dm.h
	auth.c
	bootman.c
	client.c
	ctrl.c
	daemon.c
	dm.c
	dpylist.c
	error.c
	genauth.c
	inifile.c
	netaddr.c
	process.c
	reset.c
	resource.c
	server.c
	session.c
	sessreg.c
	socket.c
	streams.c
	util.c
)
if (XDMCP)
	set(kdm_SRCS ${kdm_SRCS}
		access.c
		choose.c
		protodpy.c
		policy.c
		xdmcp.c
	)
endif (XDMCP)
if (HASXDMAUTH)
	set(kdm_SRCS ${kdm_SRCS}
		xdmauth.c
	)
endif (HASXDMAUTH)
if (K5AUTH)
	set(kdm_SRCS ${kdm_SRCS}
		krb5auth.c
	)
endif (K5AUTH)
if (SECURE_RPC)
	set(kdm_SRCS ${kdm_SRCS}
		rpcauth.c
	)
endif (SECURE_RPC)
macro_add_file_dependencies(dm.h ${confci})
macro_add_file_dependencies(error.c ${CMAKE_CURRENT_SOURCE_DIR}/printf.c)
kde4_add_executable(kdm NOGUI ${kdm_SRCS})
macro_add_compile_flags(kdm -U_REENTRANT)
target_link_libraries( kdm
	${X11_X11_LIB} ${X11_Xau_LIB} ${X11_Xdmcp_LIB} ${X11_X_EXTRA_LIBS}
	${UNIXAUTH_LIBRARIES}
	${SHADOW_LIBRARIES}
	${S_LIBRARIES}
	${KRB4_LIBRARIES}
	${KRB5_LIBRARIES}
	${COMERR_LIBRARY}
	${NSL_LIBRARIES}
	${RESOLV_LIBRARIES}
	${SOCKET_LIBRARIES}
)
if (CKCONNECTOR_FOUND)
	include_directories(${CKCONNECTOR_INCLUDE_DIR} ${DBUS_INCLUDE_DIR} ${DBUS_ARCH_INCLUDE_DIR})
	target_link_libraries( kdm ${CKCONNECTOR_LIBRARIES} ${DBUS_LIBRARIES} )
endif (CKCONNECTOR_FOUND)
if(UTIL_LIBRARIES)
	target_link_libraries( kdm ${UTIL_LIBRARIES} )
endif(UTIL_LIBRARIES)
target_link_libraries( kdm ${POSIX4_LIBRARIES} )
if(QT_RT_LIBRARY)
	target_link_libraries( kdm ${QT_RT_LIBRARY} )
endif(QT_RT_LIBRARY)

if(KDE4_ENABLE_FINAL)
        macro_add_file_dependencies(${CMAKE_CURRENT_BINARY_DIR}/kdm_final_cpp.cpp ${confci})
endif(KDE4_ENABLE_FINAL)

add_dependencies( kdm ConfigCi )

install(TARGETS kdm ${INSTALL_TARGETS_DEFAULT_ARGS})

