set(confproc ${CMAKE_SOURCE_DIR}/workspace/kdm/confproc.pl)
set(confdef ${CMAKE_SOURCE_DIR}/workspace/kdm/config.def)
set(confdoc ${CMAKE_CURRENT_SOURCE_DIR}/kdmrc-ref.docbook)

add_custom_command(
	OUTPUT ${confdoc}
	COMMAND ${PERL_EXECUTABLE} -w ${confproc} --doc ${confdef} ${confdoc}
	MAIN_DEPENDENCY ${confdef}
	DEPENDS ${confproc}
)

add_custom_target(ref DEPENDS ${confdoc})
