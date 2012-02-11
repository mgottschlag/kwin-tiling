include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcheckpass
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kdm
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace
)

########### declarative plugin ########
set(kscreenlockerplugin_SRCS
    greeter.cpp
    greeterplugin.cpp
    sessions.cpp
)

kde4_add_kcfg_files(kscreenlockerplugin_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../kcfg/kscreensaversettings.kcfgc)

qt4_automoc(${kscreenlockerplugin_SRCS})

add_library(kscreenlockerplugin SHARED ${kscreenlockerplugin_SRCS})
target_link_libraries(kscreenlockerplugin kworkspace ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY})

install(TARGETS kscreenlockerplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/kscreenlocker)


########### next target ###############
set(kscreenlocker_greet_SRCS
   greeter.cpp
   greeterapp.cpp
   main.cpp )

kde4_add_kcfg_files(kscreenlocker_greet_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../kcfg/kscreensaversettings.kcfgc)

kde4_add_executable(kscreenlocker_greet ${kscreenlocker_greet_SRCS})

target_link_libraries(kscreenlocker_greet kephal kworkspace ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTDECLARATIVE_LIBRARY} kdeclarative)

install(TARGETS kscreenlocker_greet DESTINATION ${LIBEXEC_INSTALL_DIR})

