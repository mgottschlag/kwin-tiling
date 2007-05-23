include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/workspace/plasma/plasma
                    ${CMAKE_SOURCE_DIR}/workspace/plasma/lib
                    ${CMAKE_SOURCE_DIR}/workspace/lib)

set(plasmaengineexplorer_SRCS
    engineexplorer.cpp
    main.cpp
)

kde4_add_ui_files(plasmaengineexplorer_SRCS engineexplorer.ui)
kde4_automoc(${plasmaengineexplorer_SRCS})
kde4_add_executable(plasmaengineexplorer ${plasmaengineexplorer_SRCS})

target_link_libraries(plasmaengineexplorer plasma kworkspace ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasmaengineexplorer DESTINATION ${BIN_INSTALL_DIR})

