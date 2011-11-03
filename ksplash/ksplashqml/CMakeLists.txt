project(ksplashqml)

add_subdirectory(themes)

set(ksplashqml_SRCS
    main.cpp
    SplashApp.cpp
    SplashWindow.cpp
   )

add_executable(ksplashqml ${ksplashqml_SRCS})

target_link_libraries(ksplashqml
   ${X11_LIBRARIES}
   ${QT_QTCORE_LIBRARY}
   ${QT_QTDECLARATIVE_LIBRARY}
   ${QT_QTGUI_LIBRARY}
   )

install(TARGETS ksplashqml ${INSTALL_TARGETS_DEFAULT_ARGS})
