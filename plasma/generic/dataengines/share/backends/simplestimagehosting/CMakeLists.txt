configure_file(metadata.desktop plasma-dataengine-share-addon-simplestimagehosting.desktop COPYONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-dataengine-share-addon-simplestimagehosting.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES metadata.desktop
        DESTINATION ${DATA_INSTALL_DIR}/plasma/shareprovider/simplestimagehosting/)

install(FILES contents/code/main.js
        DESTINATION ${DATA_INSTALL_DIR}/plasma/shareprovider/simplestimagehosting/contents/code/)
