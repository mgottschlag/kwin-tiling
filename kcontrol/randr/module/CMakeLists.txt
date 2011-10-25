include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/.. )

########### next target ###############

set(kded_randrmonitor_PART_SRCS
    randrmonitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../randrdisplay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../randrscreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../randroutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../randrcrtc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../randrmode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../randr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../legacyrandrscreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../ktimerdialog.cpp
    )

kde4_add_plugin(kded_randrmonitor ${kded_randrmonitor_PART_SRCS})

target_link_libraries(kded_randrmonitor ${KDE4_KDEUI_LIBS} ${X11_Xrandr_LIB} ${X11_LIBRARIES} ${KDE4_SOLID_LIBS})

install(TARGETS kded_randrmonitor  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES randrmonitor.desktop  DESTINATION  ${SERVICES_INSTALL_DIR}/kded )
