set(screenpreviewwidget_LIB_SRCS
    screenpreviewwidget.cpp
)

kde4_add_library(screenpreviewwidget SHARED ${screenpreviewwidget_LIB_SRCS})

target_link_libraries(screenpreviewwidget ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

set_target_properties(screenpreviewwidget PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

#headers are not installed, this library is used only in workspace
install(TARGETS screenpreviewwidget EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
