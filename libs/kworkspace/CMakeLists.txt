
set(kworkspace_LIB_SRCS kdisplaymanager.cpp
                        kwindowlistmenu.cpp
                        screenpreviewwidget.cpp
                        kworkspace.cpp
                        kactivityconsumer.cpp
                        kactivitycontroller.cpp
                        kactivityinfo.cpp
                        kactivitymanager_p.cpp
   )

if(SOPRANO_PLUGIN_RAPTORPARSER_FOUND AND SHAREDDESKTOPONTOLOGIES_ROOT_DIR)

soprano_add_ontology(
   kworkspace_LIB_SRCS
   "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig"
   "nfo"
   "Ontologies"
   "trig"
   )

soprano_add_ontology(
   kworkspace_LIB_SRCS
   "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nco.trig"
   "nco"
   "Ontologies"
   "trig"
   )

soprano_add_ontology(
   kworkspace_LIB_SRCS
   "${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig"
   "pimo"
   "Ontologies"
   "trig"
   )

ADD_DEFINITIONS(-DHAVE_SOPRANO_PLUGIN_RAPTORPARSER=1)

endif(SOPRANO_PLUGIN_RAPTORPARSER_FOUND AND SHAREDDESKTOPONTOLOGIES_ROOT_DIR)



set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface( kworkspace_LIB_SRCS ${ksmserver_xml} ksmserver_interface )

set(kwin_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/kwin/org.kde.KWin.xml)

set_source_files_properties(${kwin_xml} PROPERTIES INCLUDE "interface_util.h")

qt4_add_dbus_interface( kworkspace_LIB_SRCS ${kwin_xml} kwin_interface )

qt4_add_dbus_interface(
    kworkspace_LIB_SRCS
    org.kde.ActivityManager.xml
    activitymanager_interface
)

kde4_add_library(kworkspace SHARED ${kworkspace_LIB_SRCS})

target_link_libraries(kworkspace  ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES} ${KDE4_PLASMA_LIBS} )

set_target_properties(kworkspace PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} DEFINE_SYMBOL MAKE_KWORKSPACE_LIB)
install(TARGETS kworkspace EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kdisplaymanager.h
               kwindowlistmenu.h
               kworkspace.h
               kworkspace_export.h
               kactivityconsumer.h
               kactivitycontroller.h
               kactivityinfo.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kworkspace COMPONENT Devel )
