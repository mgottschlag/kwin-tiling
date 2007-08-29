set(confproc ${KDEBASE_WORKSPACE_SOURCE_DIR}/kdm/confproc.pl)
set(confdef ${KDEBASE_WORKSPACE_SOURCE_DIR}/kdm/config.def)
set(confdoc ${CMAKE_CURRENT_SOURCE_DIR}/kdmrc-ref.docbook)
add_custom_command(
	OUTPUT ${confdoc}
	COMMAND ${PERL_EXECUTABLE} -w ${confproc} --doc ${confdef} ${confdoc}
	MAIN_DEPENDENCY ${confdef}
	DEPENDS ${confproc}
)
add_custom_target(ref DEPENDS ${confdoc})
kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en)
