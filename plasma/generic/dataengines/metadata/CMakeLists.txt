project(metadata)

set(metadata_engine_SRCS metadata_engine.cpp)
 
kde4_add_plugin(plasma_engine_metadata ${metadata_engine_SRCS})

include_directories(${SOPRANO_INCLUDE_DIR})

target_link_libraries(plasma_engine_metadata
		      ${KDE4_KIO_LIBS}
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS}
                      ${NEPOMUK_LIBRARIES}
		      ${SOPRANO_LIBRARIES} )
                      
install(TARGETS plasma_engine_metadata
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
