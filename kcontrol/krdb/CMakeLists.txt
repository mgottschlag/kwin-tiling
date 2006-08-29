
add_subdirectory( themes ) 
add_subdirectory( kcs ) 
add_subdirectory( ad ) 
add_subdirectory( pics ) 

install( FILES kcmdisplayrc.upd  DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
