include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)


set(plasma-windowed_SRCS
    singleview.cpp
    main.cpp
    plasmaapp.cpp
    ${appletbrowserdialog_SRCS}
)


kde4_add_kdeinit_executable(plasma-windowed ${plasma-windowed_SRCS})

target_link_libraries(kdeinit_plasma-windowed ${KDE4_PLASMA_LIBS} kworkspace ${KDE4_KIO_LIBS} ${X11_LIBRARIES} ${KDE4_KFILE_LIBS} )


install(TARGETS kdeinit_plasma-windowed DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-windowed ${INSTALL_TARGETS_DEFAULT_ARGS})

