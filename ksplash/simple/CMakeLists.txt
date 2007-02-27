include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )

########### next target ###############

set(ksplashsimple_SRCS main.cpp )

add_executable(ksplashsimple ${ksplashsimple_SRCS})

target_link_libraries(ksplashsimple  ${X11_LIBS} )

if (X11_Xinerama_FOUND)
   target_link_libraries(ksplashsimple  ${X11_Xinerama_LIB} )
endif (X11_Xinerama_FOUND)


install(TARGETS ksplashsimple DESTINATION ${BIN_INSTALL_DIR})
