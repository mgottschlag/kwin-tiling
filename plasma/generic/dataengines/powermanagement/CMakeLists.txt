
set(powermanagement_engine_SRCS
    powermanagementengine.cpp
    powermanagementservice.cpp
    powermanagementjob.cpp
)

kde4_add_plugin(plasma_engine_powermanagement ${powermanagement_engine_SRCS})
target_link_libraries(plasma_engine_powermanagement ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_PLASMA_LIBS})
if(NOT WIN32)
target_link_libraries(plasma_engine_powermanagement kworkspace)
endif(NOT WIN32)

install(TARGETS plasma_engine_powermanagement DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-powermanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES powermanagement.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services )

