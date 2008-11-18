include_directories( BEFORE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/backend )


install_pam_service(KDM)

include(ConfigureChecks.cmake)

macro_optional_find_package(CkConnector)
if(CKCONNECTOR_FOUND)
  # when building with libck-connector, we also need the low-level D-Bus API
  find_package(DBus REQUIRED)
endif(CKCONNECTOR_FOUND)

macro_log_feature(CKCONNECTOR_FOUND "ck-connector" "The ConsoleKit connector library" "http://freedesktop.org/wiki/Software/ConsoleKit" FALSE "" "Provides ConsoleKit integration in KDM")

macro_bool_to_01(CKCONNECTOR_FOUND HAVE_CKCONNECTOR)

configure_file(config-kdm.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdm.h)

install( FILES README DESTINATION ${DATA_INSTALL_DIR}/doc/kdm )

set(confproc ${CMAKE_CURRENT_SOURCE_DIR}/confproc.pl)
set(confdef ${CMAKE_CURRENT_SOURCE_DIR}/config.def)
set(confci ${CMAKE_CURRENT_BINARY_DIR}/config.ci)
macro_additional_clean_files( ${confci} )

macro(build_confci)
	add_custom_command(
		OUTPUT ${confci}
		COMMAND ${PERL_EXECUTABLE} -w ${confproc} ${confdef} ${confci}
		MAIN_DEPENDENCY ${confdef}
		DEPENDS ${confproc}
	)
endmacro(build_confci)

build_confci()
add_custom_target(config.ci DEPENDS ${confci})

### this is *only* for the kdm home page maintainer! ###
ADD_CUSTOM_TARGET(hp scp
	${CMAKE_CURRENT_SOURCE_DIR}/README
	${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog
	${CMAKE_CURRENT_SOURCE_DIR}/TODO
	devel-home:files/kdm)

# after build_confci is defined
add_subdirectory(backend)
add_subdirectory(kfrontend)
