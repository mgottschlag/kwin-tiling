project( ksplashqml )

set(ksplashqml_SRCS
    main.cpp
   )

add_executable(ksplashqml ${ksplashqml_SRCS})

target_link_libraries(ksplashqml
   ${X11_LIBRARIES}
   ${QT_QTDECLARATIVE_LIBRARY}
   ${QT_QTUI_LIBRARY}
   )

install(TARGETS ksplashqml ${INSTALL_TARGETS_DEFAULT_ARGS})
