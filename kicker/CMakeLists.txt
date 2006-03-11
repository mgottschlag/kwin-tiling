kde4_header()

set(libkicker_core_SRCS
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/main.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/kicker.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/userrectsel.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/containerarea.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/applethandle.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/container_base.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/container_button.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/container_applet.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/container_extension.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/extensionmanager.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/menumanager.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/pluginmanager.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/showdesktop.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/containerarealayout.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/unhidetrigger.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/core/panelextension.cpp
    )

set(libkicker_buttons_SRCS
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/servicebutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/bookmarksbutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/browserbutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/desktopbutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/extensionbutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/kbutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/nonkdeappbutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/servicemenubutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/urlbutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/buttons/windowlistbutton.cpp
    )

set(libkicker_ui_SRCS
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/addbutton_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/addapplet.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/addapplet_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/appletop_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/browser_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/client_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/exe_dlg.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/k_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/quickbrowser_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/service_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/addextension_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/extensionop_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/recentapps.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/browser_dlg.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/removeapplet_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/removeextension_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/removecontainer_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/removebutton_mnu.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/hidebutton.cpp
    ${CMAKE_SOURCE_DIR}/workspace/kicker/kicker/ui/addappletvisualfeedback.cpp
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


include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = libkicker taskmanager taskbar kicker extensions applets menuext proxy data
#
