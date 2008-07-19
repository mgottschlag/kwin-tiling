include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs)

set(plasma-overlay_SRCS
    savercorona.cpp
    saverview.cpp
    main.cpp
    plasmaapp.cpp
)

add_subdirectory(containment)

set(plasmaapp_dbusXML org.kde.plasma-overlay.App.xml)
qt4_generate_dbus_interface(plasmaapp.h ${plasmaapp_dbusXML} OPTIONS -S -M)
qt4_add_dbus_adaptor(plasma-overlay_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

kde4_add_executable(plasma-overlay ${plasma-overlay_SRCS})

target_link_libraries(plasma-overlay plasma kworkspace  ${KDE4_KIO_LIBS})
if(X11_Xrender_FOUND)
  target_link_libraries(plasma-overlay ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)
set_target_properties(plasma-overlay PROPERTIES OUTPUT_NAME plasma-overlay)

install(TARGETS plasma-overlay ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-overlayrc DESTINATION ${CONFIG_INSTALL_DIR})
