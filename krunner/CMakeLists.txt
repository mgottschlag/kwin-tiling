include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

project(krunner)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/plasma/lib)

set(krunner_SRCS interface.cpp
                 runner.cpp
                 apprunner.cpp
                 restartingapplication.cpp
                 searchrunner.cpp
                 shellrunner.cpp
                 main.cpp)

qt4_add_dbus_adaptor(krunner_SRCS org.kde.krunner.Interface.xml interface.h Interface)

kde4_automoc(${krunner_SRCS})
kde4_add_executable(krunner ${krunner_SRCS})
target_link_libraries(krunner plasma ${KDE4_KDEUI_LIBS})

install(TARGETS krunner DESTINATION ${BIN_INSTALL_DIR})
install(FILES krunner.desktop  DESTINATION  ${SERVICES_INSTALL_DIR})

set(krunner_BGIMAGES images/background/dialog.svg)
install(FILES ${krunner_BGIMAGES} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/background)
