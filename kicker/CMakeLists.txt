
project(kicker)

set(libkicker_core_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/kicker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/userrectsel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/containerarea.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/applethandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/container_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/container_button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/container_applet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/container_extension.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/extensionmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/menumanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/pluginmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/showdesktop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/containerarealayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/unhidetrigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/core/panelextension.cpp
    )

set(libkicker_buttons_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/servicebutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/bookmarksbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/browserbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/desktopbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/extensionbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/kbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/nonkdeappbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/servicemenubutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/urlbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/buttons/windowlistbutton.cpp
    )

set(libkicker_ui_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/addbutton_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/addapplet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/appletop_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/browser_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/exe_dlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/k_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/quickbrowser_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/service_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/addextension_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/extensionop_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/recentapps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/browser_dlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/removeapplet_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/removeextension_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/removecontainer_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/removebutton_mnu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/hidebutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/addappletvisualfeedback.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/appletlistmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kicker/ui/appletitemdelegate.cpp
)

# Those include dirs also apply to all our subdirs
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/libkicker
   ${CMAKE_CURRENT_BINARY_DIR}/libkicker
   ${CMAKE_CURRENT_BINARY_DIR}/kicker
   ${CMAKE_SOURCE_DIR}/workspace/lib
   ${CMAKE_BINARY_DIR}/workspace/lib
)

add_subdirectory( libkicker ) 
add_subdirectory( taskmanager ) 
add_subdirectory( taskbar ) 
add_subdirectory( kicker ) 
add_subdirectory( extensions ) 
add_subdirectory( applets ) 
add_subdirectory( menuext ) 
add_subdirectory( proxy ) 
add_subdirectory( data ) 

