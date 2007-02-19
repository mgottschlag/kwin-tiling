
add_subdirectory( default ) 
add_subdirectory( standard ) 
add_subdirectory( redmond ) 
add_subdirectory( simple ) 
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(ksplashthemes_LIB_SRCS themeengine.cpp objkstheme.cpp )

kde4_automoc(${ksplashthemes_LIB_SRCS})

kde4_add_library(ksplashthemes SHARED ${ksplashthemes_LIB_SRCS} ) 

target_link_libraries(ksplashthemes  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QT3SUPPORT_LIBRARY} ${X11_X11_LIB})
if(X11_Xcursor_FOUND)
  target_link_libraries(ksplashthemes ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)

set_target_properties(ksplashthemes PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS ksplashthemes  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES ksplashplugins.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES themeengine.h objkstheme.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksplash )

