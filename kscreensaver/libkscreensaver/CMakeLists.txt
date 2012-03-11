



########### next target ###############

set(kscreensaver_LIB_SRCS main.cpp kscreensaver.cpp)


kde4_add_library(kscreensaver SHARED ${kscreensaver_LIB_SRCS})

target_link_libraries(kscreensaver  ${KDE4_KIO_LIBS} ${X11_X11_LIB})
set_target_properties(kscreensaver PROPERTIES VERSION 5.0.0 SOVERSION 5 DEFINE_SYMBOL MAKE_KSCREENSAVER_LIB)

install(TARGETS kscreensaver EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES screensaver.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

########### install files ###############

install( FILES kscreensaver.h kscreensaver_vroot.h kscreensaver_export.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

