
project(ksplash)

set(themedefault_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/themeengine/default/themelegacy.cpp )


add_subdirectory( themeengine ) 
add_subdirectory( pics ) 
add_subdirectory( kcmksplash ) 
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR}/themeengine ${CMAKE_CURRENT_SOURCE_DIR}/themeengine/default/ ${KDE4_INCLUDES} )


########### next target ###############

set(ksplash_SRCS wndmain.cpp main.cpp ${themedefault_PART_SRCS} )

qt4_add_dbus_adaptor( ksplash_SRCS org.kde.KSplash.xml wndmain.h KSplash )

kde4_automoc(${ksplash_SRCS})

kde4_add_executable(ksplash ${ksplash_SRCS})

target_link_libraries(ksplash  ${KDE4_KDEUI_LIBS} ksplashthemes ${KDE4_KIO_LIBS} )

install(TARGETS ksplash DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install( FILES ksplash.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES org.kde.KSplash.xml DESTINATION ${DBUS_INTERFACES_DIR} )
