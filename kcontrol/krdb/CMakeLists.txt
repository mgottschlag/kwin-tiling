
add_subdirectory( themes ) 
add_subdirectory( ad ) 
add_subdirectory( pics ) 

install( FILES kcmdisplayrc.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )

install(FILES krdb_libpathwipe.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update)

kde4_add_executable(krdb_clearlibrarypath krdb_clearlibrarypath.cpp)
target_link_libraries(krdb_clearlibrarypath ${KDE4_KDECORE_LIBS})
install(TARGETS krdb_clearlibrarypath DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin)
