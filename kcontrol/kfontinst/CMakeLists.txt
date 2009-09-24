if (X11_Xft_FOUND AND POLKITQT_FOUND)
    configure_file(config-fontinst.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-fontinst.h)
    include_directories(
        ${KDE4_INCLUDES}
        ${FREETYPE_INCLUDE_DIR}
        ${FONTCONFIG_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_BINARY_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus
        ${CMAKE_CURRENT_BINARY_DIR}/dbus
        ${CMAKE_CURRENT_SOURCE_DIR}/viewpart
        ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst
        ${CMAKE_CURRENT_BINARY_DIR}/kcmfontinst)
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

    set(libkfontinstdbusiface_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dbus/FontinstIface.cpp)
    set(libkfontinstview_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/FontPreview.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/PreviewSelectAction.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/viewpart/CharTip.cpp )
    set(libkfontinstjobrunner_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/JobRunner.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/ActionLabel.cpp
                                   ${CMAKE_CURRENT_SOURCE_DIR}/kcmfontinst/FontsPackage.cpp)

    add_subdirectory( lib ) 
    add_subdirectory( dbus )
    add_subdirectory( kcmfontinst ) 
    add_subdirectory( strigi-analyzer )
    add_subdirectory( apps )
    add_subdirectory( kio ) 
    add_subdirectory( thumbnail )
    add_subdirectory( viewpart )

    kde4_install_icons( ${ICON_INSTALL_DIR} )

endif (X11_Xft_FOUND AND POLKITQT_FOUND)

macro_log_feature(X11_Xft_FOUND "libxft" "X FreeType interface library" "http://www.x.org" FALSE "" "Font installer and font previews")
